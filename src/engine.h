#ifndef WINDMOLEN_ENGINE_H_
#define WINDMOLEN_ENGINE_H_


#include <stddef.h>
#include <stdint.h>

#include "move.h"
#include "options.h"
#include "position.h"
#include "thread.h"
#include "time_manager.h"



// According to official FIDE rules, after 50 consecutive reversible moves have been played, both players are allowed to
// claim a draw, but are not required to. Both players may decide to play on, resulting in legal chess positions in
// which more than 50 consecutive reversible moves have been played. However, after 75 consecutive reversible moves, the
// game ends in a draw, unless the last move was a checkmate of course.
static constexpr size_t HALFMOVE_CLOCK_LIMIT = 150;


// This structure stores any arguments that might be passed with a search command in the UCI protocol.
struct SearchArguments {
    Move search_moves[MAX_MOVES];
    size_t search_move_count;

    size_t max_search_depth;
    size_t max_search_nodes;
    size_t mate_in_x;

    bool ponder;
    bool infinite_search;
};

// The engine struct contains all parameters and resources required for finding a move. It is the only struct that
// directly interacts with the UCI protocol. It handles options and shares search parameters like time left and search
// depth with the thread pool. It also contains the position from which a search is started, and the relevant move
// history for threefold repetition checking if moves were given.
struct Engine {
    struct Options options;
    struct SearchArguments search_arguments;
    struct TimeManager time_manager;

    struct ThreadPool thread_pool;

    struct Position position;

    // When additional moves are given to the position to search from, we need to store those to be able to detect
    // threefold repetitions. Since positions can only repeat if no irreversible moves are played in between them, we
    // will store at most the maximum number of reversible moves that can be played in a row.
    struct PositionInfo info_history[HALFMOVE_CLOCK_LIMIT];
    size_t info_history_count;
};


// Sets all elements of `search_arguments` to their default values.
void reset_search_arguments(struct SearchArguments* search_arguments);

// Initializes `engine` to start position.
void initialize_engine(struct Engine* engine);

// Start the search of `engine`.
void start_search(struct Engine* engine);
// Stop the search of `engine`.
void stop_search(struct Engine* engine);

// Quit `engine`.
void quit_engine(struct Engine* engine);



#endif /* #ifndef WINDMOLEN_ENGINE_H_ */
