#include "engine.h"

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "position.h"
#include "thread.h"
#include "time_manager.h"



void reset_search_arguments(struct SearchArguments* search_arguments) {
    assert(search_arguments != NULL);

    search_arguments->search_move_count = 0;
    search_arguments->ponder            = false;

    search_arguments->max_depth = MAX_SEARCH_DEPTH;
    search_arguments->max_nodes = SIZE_MAX;
    search_arguments->mate_in_x = SIZE_MAX;
    search_arguments->infinite  = true;
}

void initialize_engine(struct Engine* engine) {
    assert(engine != NULL);

    engine->options.hash_size    = DEFAULT_HASH_SIZE;
    engine->options.thread_count = DEFAULT_THREAD_COUNT;
    engine->options.multipv      = DEFAULT_MULTIPV;

    engine->thread_pool.time_manager = &engine->time_manager;
    reset_time_manager(&engine->time_manager);

    engine->thread_pool.search_arguments = &engine->search_arguments;
    reset_search_arguments(&engine->search_arguments);

    engine->thread_pool.thread_count = 0;
    construct_thread_pool(&engine->thread_pool, engine->options.thread_count);
    setup_start_position(&engine->position, &engine->info);
}


void start_search(struct Engine* engine) {
    assert(engine != NULL);

    start_searching(&engine->thread_pool, &engine->position);
}

void stop_search(struct Engine* engine) {
    assert(engine != NULL);

    atomic_store(&engine->thread_pool.stop_search, true);
}

void quit_engine(struct Engine* engine) {
    assert(engine != NULL);

    if (!atomic_load(&engine->thread_pool.stop_search))
        stop_search(engine);

    destroy_thread_pool(&engine->thread_pool);
}
