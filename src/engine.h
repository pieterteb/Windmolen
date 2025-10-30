#ifndef WINDMOLEN_ENGINE_H_
#define WINDMOLEN_ENGINE_H_


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "move_generation.h"
#include "position.h"
#include "thread.h"



struct SearchArguments {
    Move search_moves[MAX_MOVES];
    size_t search_move_count;
    bool ponder;

    size_t moves_to_go;
    size_t max_depth;  // In plies.
    size_t max_nodes;
    size_t mate_in_x;
    uint64_t move_time;
    bool infinite;
};


// #define UCI_OPTION_NAME_MAX_LENGTH 32

// struct Option {
//     const char name[UCI_OPTION_NAME_MAX_LENGTH];
//     int type;

//     union {
//         uint64_t min;
//         uint64_t max;
//         bool on;
//     };
//     bool value;
// };

struct Engine {
    struct SearchArguments search_arguments;
    // struct Option options[1];

    struct Position position;
    struct ThreadPool thread_pool;
};


void initialize_engine(struct Engine* engine);


void start_search(struct Engine* engine);
void stop_search(struct Engine* engine);

void quit_engine(struct Engine* engine);



#endif /* #ifndef WINDMOLEN_ENGINE_H_ */
