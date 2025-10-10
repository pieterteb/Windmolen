#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <threads.h>

#include "thread.h"

#include "move_generation.h"
#include "search.h"



bool start_search_thread(struct SearchState* search_state) {
    assert(search_state != NULL);

    atomic_store(&search_state->stop_search_requested, false);
    search_state->search_completed = false;
    search_state->best_move        = 0;

    search_state->is_searching = true;

#ifndef NDEBUG
    int result = thrd_create(&search_state->thread, start_search, (void*)search_state);

    assert(result == thrd_success);
#else
    thrd_create(&search_state->thread, start_search, (void*)search_state);
#endif /* #ifndef NDEBUG */

    return true;
}

Move stop_search_thread(struct SearchState* search_state) {
    assert(search_state != NULL);

    atomic_store(&search_state->stop_search_requested, true);

#ifndef NDEBUG
    int join_result = thrd_join(search_state->thread, NULL);

    assert(join_result == thrd_success);
#else
    thrd_join(search_state->thread, NULL);
#endif /* #ifndef NDEBUG */

    search_state->is_searching = false;

    return search_state->best_move;
}
