#ifndef WINDMOLEN_SEARCH_H_
#define WINDMOLEN_SEARCH_H_


#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

#include "evaluation.h"
#include "move.h"
#include "move_generation.h"
#include "position.h"
#include "threads.h"
#include "util.h"



static constexpr size_t MAX_SEARCH_DEPTH = MAX_MOVES;


// This struct contains thread local search information.
struct Searcher {
    struct Position root_position;
    Move root_moves[MAX_MOVES];
    size_t root_move_count;

    Move principal_variation[MAX_SEARCH_DEPTH];
    size_t principal_variation_length;

    Move best_move;
    Score best_score;
    _Atomic(uint64_t) nodes_searched;

    struct ThreadPool* thread_pool;
    size_t thread_index;
};

// Returns whether `searcher` is the searcher belonging to the main thread.
static INLINE bool is_main_thread(const struct Searcher* searcher) {
    assert(searcher != nullptr);

    return searcher->thread_index == 0;
}


// Makes `searcher` search its position.
void perform_search(struct Searcher* searcher);



#endif /* #ifndef WINDMOLEN_SEARCH_H_ */
