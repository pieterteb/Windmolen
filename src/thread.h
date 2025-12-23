#ifndef WINDMOLEN_THREAD_H_
#define WINDMOLEN_THREAD_H_


#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <threads.h>

#include "options.h"
#include "position.h"
#include "search.h"
#include "time_manager.h"
#include "util.h"



// The tread structure acts as a wrapper for the searcher structure.
struct Thread {
    thrd_t handle;

    mtx_t search_mutex;
    cnd_t search_condition;

    bool searching;
    bool quit;

    struct Searcher searcher;
};

// The engine structure contains a thread pool structure. This structure contains and controls all search threads and
// acts as a bridge between the engine and the individual search threads. It also contains the time manager and search
// arguments, as these are only affecting the behaviour of the search threads. The thread pool contains one "main"
// thread. This thread takes care of search related actions that need to be executed by only one thread, for example,
// checking whether search time is exceeded and collecting the best move from other threads.
struct ThreadPool {
    struct Thread* threads;
    size_t thread_count;

    struct TimeManager* time_manager;
    struct SearchArguments* search_arguments;

    _Atomic(bool) stop_search;
    _Atomic(bool) search_aborted;
};

// Returns the main thread of `thread_pool`.
static INLINE const struct Thread* main_thread(const struct ThreadPool* thread_pool) {
    assert(thread_pool != nullptr);

    return &thread_pool->threads[0];
}


// Waits until all threads in `thread_pool` are done searching and in an idle loop.
void wait_until_finished_searching(struct ThreadPool* thread_pool, const bool wait_for_main_thread);

// Resize `thread_pool` to consist of `thread_count` different threads.
void resize_thread_pool(struct ThreadPool* thread_pool, const size_t thread_count);
// Destroys `thread_pool`.
void destroy_thread_pool(struct ThreadPool* thread_pool);

// Start a search on `root_position`. `tread_pool` gives the threads the necessary information and starts them
// individually.
void start_searching(struct ThreadPool* thread_pool, const struct Position* position);



#endif /* #ifndef WINDMOLEN_THREAD_H_ */
