#ifndef WINDMOLEN_SEARCH_H_
#define WINDMOLEN_SEARCH_H_


#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

#include "evaluation.h"
#include "move_generation.h"
#include "position.h"
#include "threads.h"
#include "types.h"



#define MAX_SEARCH_DEPTH MAX_MOVES


struct Searcher {
    struct Position root_position;
    Move root_moves[MAX_MOVES];
    size_t root_move_count;
    size_t max_search_depth;

    Move principal_variation[MAX_SEARCH_DEPTH];
    size_t principal_variation_length;

    _Atomic(Move) best_move;
    _Atomic(Score) best_score;
    atomic_uint_fast64_t nodes_searched;

    struct ThreadPool* thread_pool;
    size_t thread_index;

    bool search_aborted;
};

static inline bool is_main_thread(const struct Searcher* searcher) {
    assert(searcher != NULL);

    return searcher->thread_index == 0;
}


void start_searcher(struct Searcher* searcher);



#endif /* #ifndef WINDMOLEN_SEARCH_H_ */
