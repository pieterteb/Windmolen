#ifndef WINDMOLEN_ENGINE_H_
#define WINDMOLEN_ENGINE_H_


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "move_generation.h"
#include "options.h"
#include "position.h"
#include "thread.h"
#include "time.h"



struct SearchArguments {
    Move search_moves[MAX_MOVES];
    size_t search_move_count;
    bool ponder;

    size_t max_depth;  // In plies.
    size_t max_nodes;
    size_t mate_in_x;
    bool infinite;
};

struct Engine {
    struct Options options;
    struct SearchArguments search_arguments;
    struct TimeManager time_manager;

    struct ThreadPool thread_pool;
    struct Position position;
};


void reset_search_arguments(struct SearchArguments* search_arguments);
void initialize_engine(struct Engine* engine);

void start_search(struct Engine* engine);
void stop_search(struct Engine* engine);

void quit_engine(struct Engine* engine);



#endif /* #ifndef WINDMOLEN_ENGINE_H_ */
