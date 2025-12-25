#include "search.h"

#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "constants.h"
#include "evaluation.h"
#include "move.h"
#include "move_generation.h"
#include "position.h"
#include "thread.h"
#include "time_manager.h"
#include "uci.h"
#include "util.h"



// Stops the search if the search time is exceeded.s
static INLINE void stop_if_time_exceeded(struct Searcher* searcher) {
    assert(searcher != nullptr);
    assert(is_main_thread(searcher));

    if (get_time_us() >= searcher->thread_pool->time_manager->cutoff_time)
        atomic_store(&searcher->thread_pool->stop_search, true);
}

// Returns the searcher with the best search result.
static const struct Searcher* best_searcher(const struct ThreadPool* thread_pool) {
    assert(thread_pool != nullptr);

    const struct Searcher* best_searcher = &thread_pool->threads[0].searcher;
    const struct Searcher* searcher;
    for (size_t i = 1; i < thread_pool->thread_count; ++i) {
        searcher = &thread_pool->threads[i].searcher;
        if (searcher->best_score > atomic_load(&best_searcher->best_score))
            best_searcher = searcher;
    }

    return best_searcher;
}


// Performs alphabeta search on non-root nodes.
static Score alphabeta(struct Searcher* searcher, struct Position* position, Score alpha, const Score beta,
                       const size_t depth, const size_t ply) {
    assert(searcher != nullptr);
    assert(position != nullptr);
    assert(alpha < beta);

    atomic_fetch_add(&searcher->nodes_searched, 1);

    if (depth == 0) {
        const Score score = evaluate_position(position);

        return (position->side_to_move == COLOR_WHITE) ? score : (Score)-score;
    }

    if (is_main_thread(searcher) && !searcher->thread_pool->search_arguments->infinite_search)
        stop_if_time_exceeded(searcher);

    Move movelist[MAX_MOVES];
    const size_t move_count = generate_legal_moves(position, movelist);

    // Reset principal variation length for this depth.
    searcher->principal_variation_length[ply] = 0;

    // If there are no moves, we are mated or its stalemate.
    if (move_count == 0) {
        if (in_check(position))
            return (Score)-mate_score(ply);

        return DRAW_SCORE;
    }

    if (is_draw(position, ply))
        return DRAW_SCORE;

    Score best_score = (Score)-mate_score(ply);

    struct PositionInfo info;
    for (size_t i = 0; i < move_count; ++i) {
        do_move(position, &info, movelist[i]);
        Score score = (Score)-alphabeta(searcher, position, (Score)-beta, (Score)-alpha, depth - 1, ply + 1);
        undo_move(position, movelist[i]);

        if (score > best_score) {
            // Update the current principal variation. This is the new best move followed by the principal variation of
            // that best move. Notice that principle_variation_table[ply + 1] was already computed in the alphabeta call
            // above, so this works recursively and is well defined.
            searcher->principal_variation_table[ply][0] = movelist[i];
            memcpy(&searcher->principal_variation_table[ply][1], &searcher->principal_variation_table[ply + 1][0],
                   searcher->principal_variation_length[ply + 1] * sizeof(Move));
            searcher->principal_variation_length[ply] = searcher->principal_variation_length[ply + 1] + 1;

            // Cut node.
            if (score >= beta)
                return score;

            best_score = score;

            if (score > alpha)
                alpha = score;
        }

        if (atomic_load(&searcher->thread_pool->stop_search)) {
            if (is_main_thread(searcher))
                atomic_store(&searcher->thread_pool->search_aborted, true);
            break;
        }
    }

    return best_score;
}

// Performs search on root nodes.
static Score root_search(struct Searcher* searcher, const size_t depth, size_t* best_move_index) {
    assert(searcher != nullptr);
    assert(best_move_index != nullptr);
    assert(depth > 0);

    atomic_fetch_add(&searcher->nodes_searched, 1);

    Score best_score     = (Score)-mate_score(0);
    Score alpha          = MIN_SCORE;
    constexpr Score beta = MAX_SCORE;

    struct PositionInfo info;
    for (size_t i = 0; i < searcher->root_move_count; ++i) {
        do_move(&searcher->root_position, &info, searcher->root_moves[i]);
        Score score = (Score)-alphabeta(searcher, &searcher->root_position, -beta, (Score)-alpha, depth - 1, 1);
        undo_move(&searcher->root_position, searcher->root_moves[i]);

        if (score > best_score && !atomic_load(&searcher->thread_pool->search_aborted)) {
            // Update the current principal variation. This is the new best move followed by the principal variation of
            // that best move. Notice that principle_variation_table[1] was already computed in the alphabeta call
            // above, so this works recursively and is well defined.
            searcher->principal_variation_table[0][0] = searcher->root_moves[i];
            memcpy(&searcher->principal_variation_table[0][1], &searcher->principal_variation_table[1][0],
                   searcher->principal_variation_length[1] * sizeof(Move));
            searcher->principal_variation_length[0] = searcher->principal_variation_length[1] + 1;

            best_score       = score;
            *best_move_index = i;

            if (score > alpha)
                alpha = score;
        }

        if (atomic_load(&searcher->thread_pool->stop_search))
            break;
    }

    return best_score;
}


// Collects info from `thread_pool` and prints this to UCI together with `depth`, `multipv` and `elapsed_time`.
static void long_info(const struct ThreadPool* thread_pool, const size_t depth, const size_t multipv,
                      const uint64_t elapsed_time) {
    assert(thread_pool != nullptr);
    assert(depth > 0);
    assert(multipv > 0);
    assert(elapsed_time > 0);

    size_t nodes_searched = 0;
    for (size_t i = 0; i < thread_pool->thread_count; ++i)
        nodes_searched += atomic_load(&thread_pool->threads[i].searcher.nodes_searched);

    const struct Searcher* winner = best_searcher(thread_pool);

    uci_long_info(depth, multipv, winner->best_score, nodes_searched, elapsed_time,
                  winner->principal_variation_table[0], winner->principal_variation_length[0]);
}

// Make `searcher` perform iterative deepening.
static void iterative_deepening(struct Searcher* searcher) {
    assert(searcher != nullptr);

    const uint64_t start_time = get_time_us();
    const size_t max_depth    = searcher->thread_pool->search_arguments->max_search_depth;

    for (size_t depth = 1; depth <= max_depth; ++depth) {
        size_t best_move_index = SIZE_MAX;
        const Score best_score = root_search(searcher, depth, &best_move_index);

        // If we have completely searched at least one root move, we update the best score. Else, the search result
        // can not be trusted.
        if (best_move_index != SIZE_MAX) {
            atomic_store(&searcher->best_score, best_score);

            // Make sure the new best move is checked first in the next iteration.
            searcher->root_moves[best_move_index] = searcher->root_moves[0];
            searcher->root_moves[0]               = searcher->principal_variation_table[0][0];
        }

        if (is_main_thread(searcher)) {
            const uint64_t elapsed_time = get_time_us() - start_time;
            long_info(searcher->thread_pool, depth, 1, elapsed_time);

            // We stop if we have searched too many nodes or we have found mate.
            if (searcher->nodes_searched > searcher->thread_pool->search_arguments->max_search_nodes
                || is_mate_score(best_searcher(searcher->thread_pool)->best_score))
                atomic_store(&searcher->thread_pool->stop_search, true);
        }

        if (atomic_load(&searcher->thread_pool->stop_search))
            break;
    }
}


void perform_search(struct Searcher* searcher) {
    assert(searcher != nullptr);

    iterative_deepening(searcher);

    if (!is_main_thread(searcher))
        return;

    // If we are searching in ponder mode or with infinite depth, we must not output a best move before the stop command
    // as stated by the UCI protocol.
    const bool stop_required = searcher->thread_pool->search_arguments->ponder
                            || searcher->thread_pool->search_arguments->infinite_search;
    while (!atomic_load(&searcher->thread_pool->stop_search) && stop_required) {}

    // Other threads might still be stopping their search which can cause and incorrect results in
    // uci_best_move(). Therefore, we wait until all threads except for the main thread (as the main thread is right
    // here) finished searching.
    wait_until_finished_searching(searcher->thread_pool, false);

    uci_best_move(best_move(best_searcher(searcher->thread_pool)));
}
