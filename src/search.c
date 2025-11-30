#include "search.h"

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "evaluation.h"
#include "move_generation.h"
#include "position.h"
#include "thread.h"
#include "time_manager.h"
#include "uci.h"



static inline void stop_if_time_exceeded(struct Searcher* searcher) {
    assert(searcher != NULL);
    assert(is_main_thread(searcher));

    if (get_time_us() >= searcher->thread_pool->time_manager->cutoff_time)
        atomic_store(&searcher->thread_pool->stop_search, true);
}

static const struct Searcher* best_searcher(const struct ThreadPool* thread_pool) {
    assert(thread_pool != NULL);

    const struct Searcher* best_searcher = &thread_pool->threads[0].searcher;
    const struct Searcher* searcher;
    for (size_t i = 1; i < thread_pool->thread_count; ++i) {
        searcher = &thread_pool->threads[i].searcher;
        if (&best_searcher->best_score < &searcher->best_score)
            best_searcher = searcher;
    }

    return best_searcher;
}


static Score negamax(struct Searcher* searcher, struct Position* position, size_t depth, size_t ply) {
    assert(searcher != NULL);
    assert(position != NULL);

    ++searcher->nodes_searched;

    if (depth == 0) {
        Score score = evaluate_position(position);
        return (position->side_to_move == COLOR_WHITE) ? score : -score;
    }

    if (is_main_thread(searcher) && !searcher->thread_pool->search_arguments->infinite)
        stop_if_time_exceeded(searcher);

    Move movelist[MAX_MOVES];
    size_t move_count = generate_legal_moves(position, movelist);

    // Check if it is a stalemate position. If move_count == 0 but it ís check, it is mate which will be handled
    // automatically after this if statement. Also check the 50-move-rule and threefold repetition.
    if ((move_count == 0 && !in_check(position)) || is_draw(position, ply))
        return DRAWN_SCORE;

    Score best_score = -MATE_SCORE + (Score)ply;
    struct PositionInfo info;
    for (size_t i = 0; i < move_count; ++i) {
        do_move(position, &info, movelist[i]);

        Score score = -negamax(searcher, position, depth - 1, ply + 1);
        best_score  = (score > best_score) ? score : best_score;

        undo_move(position, movelist[i]);

        if (atomic_load(&searcher->thread_pool->stop_search)) {
            searcher->search_aborted = true;
            break;
        }
    }

    return best_score;
}

static Score root_search(struct Searcher* searcher, size_t depth, size_t* best_move_index) {
    assert(searcher != NULL);
    assert(best_move_index != NULL);
    assert(depth > 0);

    ++searcher->nodes_searched;

    Score best_score = -MATE_SCORE;
    struct PositionInfo info;
    for (size_t i = 0; i < searcher->root_move_count; ++i) {
        do_move(&searcher->root_position, &info, searcher->root_moves[i]);

        Score score = -negamax(searcher, &searcher->root_position, depth - 1, 1);
        if (score > best_score && !searcher->search_aborted) {
            best_score       = score;
            *best_move_index = i;
        }

        undo_move(&searcher->root_position, searcher->root_moves[i]);

        if (atomic_load(&searcher->thread_pool->stop_search))
            break;
    }

    return best_score;
}

static void iterative_deepening(struct Searcher* searcher) {
    assert(searcher != NULL);

    uint64_t start_time = get_time_us();
    size_t max_depth    = searcher->thread_pool->search_arguments->max_depth;

    for (size_t depth = 1; depth <= max_depth; ++depth) {
        size_t best_move_index = SIZE_MAX;
        Score best_score       = root_search(searcher, depth, &best_move_index);

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
            uint64_t elapsed_time = get_time_us() - start_time;
            uci_long_info(depth, 1, searcher->best_score, searcher->nodes_searched, elapsed_time, searcher->best_move);

            if (searcher->nodes_searched > searcher->thread_pool->search_arguments->max_nodes)
                atomic_store(&searcher->thread_pool->stop_search, true);
        }

        if (atomic_load(&searcher->thread_pool->stop_search))
            break;
    }
}


void start_searcher(struct Searcher* searcher) {
    assert(searcher != NULL);

    iterative_deepening(searcher);

    if (!is_main_thread(searcher))
        return;

    // If we are searching in ponder mode or with infinite depth, we must not output a best move before the stop command
    // as stated by the UCI protocol.
    while (!searcher->thread_pool->stop_search
           && (searcher->thread_pool->search_arguments->ponder || searcher->thread_pool->search_arguments->infinite)) {}

    const struct Searcher* winning_searcher = best_searcher(searcher->thread_pool);

    uci_best_move(winning_searcher->best_move);
}
