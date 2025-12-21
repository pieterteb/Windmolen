#include "search.h"

#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

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
        if (searcher->best_score > best_searcher->best_score)
            best_searcher = searcher;
    }

    return best_searcher;
}


// Performs alphabeta search on non-root nodes.
static Score alphabeta(struct Searcher* searcher, struct Position* position, Score alpha, Score beta,
                       const size_t depth, const size_t ply) {
    assert(searcher != nullptr);
    assert(position != nullptr);

    atomic_store(&searcher->nodes_searched, atomic_load(&searcher->nodes_searched) + 1);

    if (depth == 0) {
        Score score = evaluate_position(position);
        return (position->side_to_move == COLOR_WHITE) ? score : -score;
    }

    if (is_main_thread(searcher) && !searcher->thread_pool->search_arguments->infinite_search)
        stop_if_time_exceeded(searcher);

    Move movelist[MAX_MOVES];
    size_t move_count = generate_legal_moves(position, movelist);

    // Check if it is a stalemate position. If move_count == 0 but it ís check, it is mate which will be handled
    // automatically after this if statement. Also check the 50-move-rule and threefold repetition.
    if ((move_count == 0 && !in_check(position)) || is_draw(position, ply))
        return DRAW_SCORE;

    Score best_score = -MATE_SCORE + (Score)ply;
    struct PositionInfo info;
    for (size_t i = 0; i < move_count; ++i) {
        do_move(position, &info, movelist[i]);
        Score score = -alphabeta(searcher, position, -beta, -alpha, depth - 1, ply + 1);
        undo_move(position, movelist[i]);

        if (score > best_score) {
            best_score = score;
            if (score > alpha)
                alpha = score;
        }
        if (score >= beta)
            return best_score;

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

    atomic_store(&searcher->nodes_searched, atomic_load(&searcher->nodes_searched) + 1);

    Score best_score = -MATE_SCORE;
    struct PositionInfo info;
    for (size_t i = 0; i < searcher->root_move_count; ++i) {
        do_move(&searcher->root_position, &info, searcher->root_moves[i]);

        Score score = -alphabeta(searcher, &searcher->root_position, -MATE_SCORE, MATE_SCORE, depth - 1, 1);
        if (score > best_score && !atomic_load(&searcher->thread_pool->search_aborted)) {
            best_score       = score;
            *best_move_index = i;
        }

        undo_move(&searcher->root_position, searcher->root_moves[i]);

        if (atomic_load(&searcher->thread_pool->stop_search))
            break;
    }

    return best_score;
}


// Collects info from `thread_pool` and prints this to UCI together with `depth`, `multipv` and `elapsed_time`.
static void long_info(const struct ThreadPool* thread_pool, size_t depth, size_t multipv, uint64_t elapsed_time) {
    assert(thread_pool != nullptr);
    assert(depth > 0);
    assert(multipv > 0);
    assert(elapsed_time > 0);

    size_t nodes_searched = 0;
    for (size_t i = 0; i < thread_pool->thread_count; ++i)
        nodes_searched += atomic_load(&thread_pool->threads[i].searcher.nodes_searched);

    const struct Searcher* winner = best_searcher(thread_pool);

    uci_long_info(depth, multipv, winner->best_score, nodes_searched, elapsed_time, winner->best_move);
}

// Make `searcher` perform iterative deepening.
static void iterative_deepening(struct Searcher* searcher) {
    assert(searcher != nullptr);

    const uint64_t start_time = get_time_us();
    const size_t max_depth    = searcher->thread_pool->search_arguments->max_search_depth;

    for (size_t depth = 1; depth <= max_depth; ++depth) {
        size_t best_move_index = SIZE_MAX;
        const Score best_score = root_search(searcher, depth, &best_move_index);

        // If at least one move has been completely searched, we update the current best move. Else, the search result
        // can not be trusted.
        if (best_move_index != SIZE_MAX) {
            searcher->best_score = best_score;
            searcher->best_move  = searcher->root_moves[best_move_index];

            // Make sure the new best move is checked first in the next iteration.
            searcher->root_moves[best_move_index] = searcher->root_moves[0];
            searcher->root_moves[0]               = searcher->best_move;
        }

        if (is_main_thread(searcher)) {
            const uint64_t elapsed_time = get_time_us() - start_time;
            long_info(searcher->thread_pool, depth, 1, elapsed_time);

            if (searcher->nodes_searched > searcher->thread_pool->search_arguments->max_search_nodes)
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

    uci_best_move(best_searcher(searcher->thread_pool)->best_move);
}
