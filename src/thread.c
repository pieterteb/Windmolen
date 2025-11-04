#include "thread.h"

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <threads.h>

#include "engine.h"
#include "move_generation.h"
#include "options.h"
#include "search.h"
#include "time_manager.h"



static void wait_until_thread_finished_searching(struct Thread* thread) {
    assert(thread != NULL);

    mtx_lock(&thread->search_mutex);

    while (thread->searching)
        cnd_wait(&thread->search_condition, &thread->search_mutex);

    mtx_unlock(&thread->search_mutex);
}

static void wait_until_finished_searching(struct ThreadPool* threadpool) {
    assert(threadpool != NULL);

    for (size_t i = 0; i < threadpool->thread_count; ++i)
        wait_until_thread_finished_searching(&threadpool->threads[i]);
}

static int thread_loop(void* thread_) {
    assert(thread_ != NULL);

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

        start_searcher(&thread->searcher);
    }

    return thrd_success;
}

static void construct_thread(struct Thread* thread) {
    assert(thread != NULL);

    thrd_create(&thread->handle, thread_loop, thread);

    mtx_init(&thread->search_mutex, mtx_plain);
    cnd_init(&thread->search_condition);

    thread->quit     = false;
    thread->searcher = (struct Searcher){0};

    wait_until_thread_finished_searching(thread);
}

static void destroy_thread(struct Thread* thread) {
    assert(thread != NULL);

    mtx_lock(&thread->search_mutex);

    thread->quit      = true;
    thread->searching = true;
    cnd_signal(&thread->search_condition);

    mtx_unlock(&thread->search_mutex);

    thrd_join(thread->handle, /* Ignore thread result. */ NULL);

    mtx_destroy(&thread->search_mutex);
    cnd_destroy(&thread->search_condition);
}

static void start_search_thread(struct Thread* thread) {
    assert(thread != NULL);

    mtx_lock(&thread->search_mutex);

    thread->searching = true;
    cnd_signal(&thread->search_condition);

    mtx_unlock(&thread->search_mutex);
}

void construct_thread_pool(struct ThreadPool* thread_pool, size_t thread_count) {
    assert(thread_pool != NULL);
    assert(thread_count > 0 && thread_count <= MAX_THREAD_COUNT);
    // Either this is the first time we construct the thread pool or we should not be searching.
    assert(thread_pool->stop_search || thread_pool->thread_count == 0);

    if (thread_pool->thread_count > 0)
        wait_until_finished_searching(thread_pool);
    thread_pool->stop_search = true;

    while (thread_count > thread_pool->thread_count)
        construct_thread(&thread_pool->threads[thread_pool->thread_count++]);

    while (thread_count < thread_pool->thread_count)
        destroy_thread(&thread_pool->threads[thread_pool->thread_count--]);
}

void destroy_thread_pool(struct ThreadPool* thread_pool) {
    assert(thread_pool != NULL);
    assert(atomic_load(&thread_pool->stop_search) == true);

    wait_until_finished_searching(thread_pool);

    for (size_t i = 0; i < thread_pool->thread_count; ++i)
        destroy_thread(&thread_pool->threads[i]);
}

void start_searching(struct ThreadPool* thread_pool, struct Position* root_position) {
    assert(thread_pool != NULL);
    assert(root_position != NULL);

    wait_until_finished_searching(thread_pool);

    thread_pool->stop_search = false;
    if (!thread_pool->search_arguments->infinite)
        update_time_manager(thread_pool->time_manager, root_position->side_to_move);

    Move root_moves[MAX_MOVES];
    size_t root_move_count;
    if (thread_pool->search_arguments->search_move_count != 0) {
        memcpy(root_moves, thread_pool->search_arguments->search_moves,
               thread_pool->search_arguments->search_move_count * sizeof(*thread_pool->search_arguments->search_moves));
        root_move_count = thread_pool->search_arguments->search_move_count;
    } else {
        root_move_count = generate_legal_moves(root_position, root_moves);
    }

    struct Thread* thread;
    for (size_t i = 0; i < thread_pool->thread_count; ++i) {
        thread = &thread_pool->threads[i];

        thread->searcher.root_position = *root_position;
        memcpy(thread->searcher.root_moves, root_moves, root_move_count * sizeof(*thread->searcher.root_moves));
        thread->searcher.root_move_count = root_move_count;

        thread->searcher.principal_variation_length = 0;

        thread->searcher.best_move      = root_moves[0];
        thread->searcher.best_score     = -MATE_SCORE;
        thread->searcher.nodes_searched = 0;

        thread->searcher.thread_pool  = thread_pool;
        thread->searcher.thread_index = i;

        thread->searcher.search_aborted = false;

        start_search_thread(thread);
    }
}
