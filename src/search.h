#ifndef WINDMOLEN_SEARCH_H_
#define WINDMOLEN_SEARCH_H_


#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

#include "constants.h"
#include "move.h"
#include "position.h"
#include "score.h"
#include "threads.h"
#include "util.h"



// This struct contains thread local search information.
struct Searcher {
    struct Position root_position;
    Move root_moves[MAX_MOVES];
    size_t root_move_count;

    int8_t root_move_values[MAX_MOVES];
    size_t sorted_until_index;

    // principal_variation_table[i][j] is the jth move of the principle variation at depth i. We have 0 <= j <=
    // principle_variation_length[i].
    Move principal_variation_table[MAX_SEARCH_DEPTH][MAX_SEARCH_DEPTH];
    size_t principal_variation_length[MAX_SEARCH_DEPTH];

    _Atomic(Value) best_value;
    _Atomic(uint64_t) nodes_searched;

    struct ThreadPool* thread_pool;
    size_t thread_index;
};

// Returns whether `searcher` is the searcher belonging to the main thread.
static INLINE bool is_main_thread(const struct Searcher* searcher) {
    assert(searcher != nullptr);

    return searcher->thread_index == 0;
}

// Returns the best move of `searcher`.
static INLINE Move best_move(const struct Searcher* searcher) {
    assert(searcher != nullptr);

    // This move is guaranteed to exist by definition of start_searching().
    return searcher->principal_variation_table[0][0];
}


// Makes `searcher` search its position.
void perform_search(struct Searcher* searcher);



#endif /* #ifndef WINDMOLEN_SEARCH_H_ */
