#ifndef WINDMOLEN_THREAD_H_
#define WINDMOLEN_THREAD_H_


#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <threads.h>

#include "evaluation.h"
#include "move_generation.h"
#include "options.h"
#include "search.h"
#include "time_manager.h"



struct Thread {
    thrd_t handle;

    mtx_t search_mutex;
    cnd_t search_condition;

    bool searching;
    bool quit;

    struct Searcher searcher;
};

struct ThreadPool {
    size_t thread_count;
    struct Thread threads[MAX_THREAD_COUNT];

    struct TimeManager* time_manager;
    struct SearchArguments* search_arguments;

    _Atomic(bool) stop_search;
};

static inline struct Thread* main_thread(struct ThreadPool* thread_pool) {
    assert(thread_pool != NULL);

    return &thread_pool->threads[0];
}


void construct_thread_pool(struct ThreadPool* thread_pool, size_t thread_count);
void destroy_thread_pool(struct ThreadPool* thread_pool);

void start_searching(struct ThreadPool* thread_pool, struct Position* position);



#endif /* #ifndef WINDMOLEN_THREAD_H_ */
