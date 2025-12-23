#include "engine.h"

#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>

#include "constants.h"
#include "options.h"
#include "position.h"
#include "thread.h"
#include "time_manager.h"



void reset_search_arguments(struct SearchArguments* search_arguments) {
    assert(search_arguments != nullptr);

    constexpr bool DEFAULT_PONDER_MODE        = false;
    constexpr size_t DEFAULT_MAX_SEARCH_DEPTH = MAX_SEARCH_DEPTH;
    constexpr size_t DEFAULT_MAX_SEARCH_NODES = SIZE_MAX;
    constexpr size_t DEFAULT_MATE_IN_X        = 0;
    constexpr size_t DEFAULT_INFINITE_SEARCH  = true;


    search_arguments->search_move_count = 0;

    search_arguments->max_search_depth = DEFAULT_MAX_SEARCH_DEPTH;
    search_arguments->max_search_nodes = DEFAULT_MAX_SEARCH_NODES;
    search_arguments->mate_in_x        = DEFAULT_MATE_IN_X;

    search_arguments->ponder          = DEFAULT_PONDER_MODE;
    search_arguments->infinite_search = DEFAULT_INFINITE_SEARCH;
}


void initialize_engine(struct Engine* engine) {
    assert(engine != nullptr);

    initialize_options(&engine->options);
    reset_time_manager(&engine->time_manager);
    reset_search_arguments(&engine->search_arguments);

    engine->thread_pool.time_manager     = &engine->time_manager;
    engine->thread_pool.search_arguments = &engine->search_arguments;

    // We need to make sure the thread pool starts with 0 threads to properly resize the thread pool.
    engine->thread_pool.thread_count = 0;
    resize_thread_pool(&engine->thread_pool, engine->options.thread_count);

    // We default to the regular start position of chess.
    engine->info_history_count = 0;
    setup_start_position(&engine->position, &engine->info_history[engine->info_history_count]);
}


void start_search(struct Engine* engine) {
    assert(engine != nullptr);

    start_searching(&engine->thread_pool, &engine->position);
}

void stop_search(struct Engine* engine) {
    assert(engine != nullptr);

    atomic_store(&engine->thread_pool.stop_search, true);
}

void quit_engine(struct Engine* engine) {
    assert(engine != nullptr);

    stop_search(engine);

    destroy_thread_pool(&engine->thread_pool);
}
