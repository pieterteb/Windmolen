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



static void stop_if_time_exceeded(struct Searcher* searcher) {
    assert(searcher != NULL);
    assert(is_main_thread(searcher));

    if (get_time_us() >= searcher->thread_pool->time_manager.cutoff_time)
        atomic_store(&searcher->thread_pool->stop_search, true);
}

static Score negamax(struct Searcher* searcher, struct Position* position, size_t depth, _Atomic(Move)* best_move) {
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
    size_t move_count;
    if (depth == searcher->max_search_depth) {
        memcpy(movelist, searcher->root_moves, searcher->root_move_count * sizeof(Move));
        move_count = searcher->root_move_count;
    } else {
        move_count = generate_legal_moves(position, movelist);
    }

    if (move_count == 0) {
        Score score;
        if (position->checkers[position->side_to_move] != 0) {
            score = -MATE_SCORE + (Score)(searcher->max_search_depth - depth);
        } else {
            score = DRAWN_SCORE + (Score)(searcher->max_search_depth - depth);
        }
        return score;
    }

    Score max_score = -MAX_SCORE;
    for (size_t i = 0; i < move_count; ++i) {
        if (atomic_load(&searcher->thread_pool->stop_search))
            return max_score;

        struct Position new_position = *position;
        do_move(&new_position, movelist[i]);

        Score score = -negamax(searcher, &new_position, depth - 1, best_move);
        if (score > max_score) {
            max_score = score;

            if (depth == searcher->max_search_depth)
                atomic_store(best_move, movelist[i]);
        }
    }

    return max_score;
}

static void iterative_deepening(struct Searcher* searcher) {
    assert(searcher != NULL);

    size_t max_depth = searcher->thread_pool->search_arguments->max_depth;

    for (size_t depth = 1; depth <= max_depth; ++depth) {
        searcher->max_search_depth = depth;

        atomic_store(&searcher->best_score, negamax(searcher, &searcher->root_position, depth, &searcher->best_move));

        uci_long_info(depth, 1, atomic_load(&searcher->best_score), atomic_load(&searcher->nodes_searched),
                      atomic_load(&searcher->best_move));

        if (atomic_load(&searcher->thread_pool->stop_search))
            break;
    }
}

static const struct Searcher* best_searcher(const struct ThreadPool* thread_pool) {
    assert(thread_pool != NULL);

    const struct Searcher* best_searcher = &thread_pool->threads[0].searcher;
    const struct Searcher* searcher;
    for (size_t i = 1; i < thread_pool->thread_count; ++i) {
        searcher = &thread_pool->threads[i].searcher;
        if (atomic_load(&best_searcher->best_score) < atomic_load(&searcher->best_score))
            best_searcher = searcher;
    }

    return best_searcher;
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
