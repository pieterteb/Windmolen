#include "thread.h"

#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <string.h>
#include <threads.h>

#include "engine.h"
#include "move.h"
#include "move_generation.h"
#include "search.h"
#include "time_manager.h"



// Waits until `thread` has signaled that it is done searching and in an idle loop.
static void wait_until_thread_finished_searching(struct Thread* thread) {
    assert(thread != nullptr);

    mtx_lock(&thread->search_mutex);

    while (thread->searching)
        cnd_wait(&thread->search_condition, &thread->search_mutex);

    mtx_unlock(&thread->search_mutex);
}

// Waits until all threads in `thread_pool` are done searching and in an idle loop.
static void wait_until_finished_searching(struct ThreadPool* thread_pool) {
    assert(thread_pool != nullptr);

    for (size_t i = 0; i < thread_pool->thread_count; ++i)
        wait_until_thread_finished_searching(&thread_pool->threads[i]);
}

// The main thread loop. This is the function that gets executed when starting a new thread (`thread_`). The thread
// stays in a waiting loop until it is signaled by the engine thread that it needs to start searching. It will then
// start its searcher. When `start_searcher` has finished, the thread returns to the waiting loop and awaits a new
// search prompt. If the thread has been told to quit, it exits the infinite loop and terminates.
static int thread_loop(void* thread_) {
    assert(thread_ != nullptr);

    struct Thread* thread = (struct Thread*)thread_;

    while (true) {
        mtx_lock(&thread->search_mutex);

        thread->searching = false;
        cnd_signal(&thread->search_condition);
        while (!thread->searching)
            cnd_wait(&thread->search_condition, &thread->search_mutex);

        if (thread->quit)
            break;

        mtx_unlock(&thread->search_mutex);

        perform_search(&thread->searcher);
    }

    return thrd_success;
}

// Starts a new thread and stores its info in `thread`.
static void construct_thread(struct Thread* thread) {
    assert(thread != nullptr);

    thrd_create(&thread->handle, thread_loop, thread);

    mtx_init(&thread->search_mutex, mtx_plain);
    cnd_init(&thread->search_condition);

    thread->quit = false;

    // Make sure the thread is in the idle loop before returning.
    wait_until_thread_finished_searching(thread);
}

// Destroys `thread`.
static void destroy_thread(struct Thread* thread) {
    assert(thread != nullptr);

    mtx_lock(&thread->search_mutex);

    // Tell the thread to leave the idle loop and exit the main loop. This terminates the thread.
    thread->quit      = true;
    thread->searching = true;
    cnd_signal(&thread->search_condition);

    mtx_unlock(&thread->search_mutex);

    thrd_join(thread->handle, /* Ignore thread result. */ nullptr);

    mtx_destroy(&thread->search_mutex);
    cnd_destroy(&thread->search_condition);
}

// Signal `thread` that it should start searching.
static void start_search_thread(struct Thread* thread) {
    assert(thread != nullptr);
    assert(!thread->searching);

    mtx_lock(&thread->search_mutex);

    thread->searching = true;
    cnd_signal(&thread->search_condition);

    mtx_unlock(&thread->search_mutex);
}


void resize_thread_pool(struct ThreadPool* thread_pool, const size_t thread_count) {
    assert(thread_pool != nullptr);
    assert(thread_count > 0 && thread_count <= OPTION_THREAD_COUNT_MAX);
    // Either this is the first time we construct the thread pool or we should not be searching.
    assert(atomic_load(&thread_pool->stop_search) || thread_pool->thread_count == 0);

    wait_until_finished_searching(thread_pool);

    while (thread_count > thread_pool->thread_count)
        construct_thread(&thread_pool->threads[thread_pool->thread_count++]);

    while (thread_count < thread_pool->thread_count)
        destroy_thread(&thread_pool->threads[thread_pool->thread_count--]);
}

void destroy_thread_pool(struct ThreadPool* thread_pool) {
    assert(thread_pool != nullptr);
    assert(atomic_load(&thread_pool->stop_search));

    wait_until_finished_searching(thread_pool);

    for (size_t i = 0; i < thread_pool->thread_count; ++i)
        destroy_thread(&thread_pool->threads[i]);
}


void start_searching(struct ThreadPool* thread_pool, const struct Position* root_position) {
    assert(thread_pool != nullptr);
    assert(root_position != nullptr);

    // Make sure all threads are idle. This is necessary in case a search is stopped and another search is started
    // quickly after that.
    wait_until_finished_searching(thread_pool);

    thread_pool->stop_search                 = false;
    thread_pool->search_aborted              = false;
    struct SearchArguments* search_arguments = thread_pool->search_arguments;

    Move root_moves[MAX_MOVES];
    size_t root_move_count;
    if (search_arguments->search_move_count != 0) {
        memcpy(root_moves, search_arguments->search_moves,
               search_arguments->search_move_count * sizeof(*search_arguments->search_moves));
        root_move_count = search_arguments->search_move_count;
    } else {
        root_move_count = generate_legal_moves(root_position, root_moves);
    }

    if (!search_arguments->infinite_search)
        update_time_manager(thread_pool->time_manager, root_position->side_to_move);

    struct Searcher* searcher;
    for (size_t i = 0; i < thread_pool->thread_count; ++i) {
        searcher = &thread_pool->threads[i].searcher;

        memcpy(&searcher->root_position, root_position, sizeof(*root_position));
        memcpy(searcher->root_moves, root_moves, root_move_count * sizeof(*searcher->root_moves));
        searcher->root_move_count = root_move_count;

        searcher->principal_variation_length = 0;

        // We set best_move to the first move such that we always have a move to return in case of short search times.
        searcher->best_move      = root_moves[0];
        searcher->best_score     = -MATE_SCORE;
        searcher->nodes_searched = 0;

        searcher->thread_pool  = thread_pool;
        searcher->thread_index = i;

        start_search_thread(&thread_pool->threads[i]);
    }
}
