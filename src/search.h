#ifndef SEARCH_H
#define SEARCH_H


#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <threads.h>

#include "move_generation.h"
#include "position.h"



struct SearchState {
    thrd_t thread;
    atomic_bool stop_search_requested;
    Move best_move;
    bool search_completed;
    bool is_searching;

    Move movelist[MAX_MOVES];
    size_t move_count;
};


int start_search(void* search_state_void);



#endif /* SEARCH_H */
