#include "engine.h"

#include <assert.h>
#include <stdatomic.h>

#include "thread.h"



void initialize_engine(struct Engine* engine) {
    assert(engine != NULL);

    engine->thread_pool.search_arguments = &engine->search_arguments;
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
