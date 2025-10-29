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
#include "uci.h"



static Score negamax(struct Searcher* searcher, size_t depth, Move movelist[MAX_MOVES], size_t move_count,
                     Move movestack[MAX_SEARCH_DEPTH], size_t movestack_index) {
    assert(searcher != NULL);

    ++searcher->nodes_searched;

    if (depth == 0) {
        Score score = evaluate_position(&searcher->root_position, move_count, searcher->max_search_depth);
        return (searcher->root_position.side_to_move == COLOR_WHITE) ? score : -score;
    }

    Score max_score = -MAX_SCORE;
    if (depth == searcher->max_search_depth)
        max_score = searcher->best_score;

    Move new_movelist[MAX_MOVES];
    for (size_t i = 0; i < move_count; ++i) {
        struct Position copy = searcher->root_position;
        do_move(&searcher->root_position, movelist[i]);
        movestack[movestack_index++] = movelist[i];

        size_t new_move_count = generate_legal_moves(&searcher->root_position, new_movelist);

        Score score = -negamax(searcher, depth - 1, new_movelist, new_move_count, movestack, movestack_index);
        if (score > max_score) {
            max_score = score;

            if (depth == searcher->max_search_depth) {
                searcher->best_score = max_score;
                searcher->best_move  = movelist[i];

                memcpy(searcher->move_stack, movestack, searcher->max_search_depth * sizeof(Move));
                searcher->move_stack_count = searcher->max_search_depth;
            }
        }

        searcher->root_position = copy;
        --movestack_index;

        if (atomic_load(&searcher->thread_pool->stop_search))
            return MAX_SCORE;
    }

    return max_score;
}

static void iterative_deepening(struct Searcher* searcher) {
    assert(searcher != NULL);

    size_t max_depth = searcher->thread_pool->search_arguments->max_depth;

    for (size_t depth = 1; depth <= max_depth; ++depth) {
        searcher->max_search_depth = depth;

        negamax(searcher, depth, searcher->root_moves, searcher->root_move_count, searcher->move_stack, 0);

        uci_long_info(depth, 1, searcher->best_score, searcher->nodes_searched, searcher->move_stack,
                      searcher->move_stack_count);

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
