#ifndef WINDMOLEN_MOVE_GENERATION_H_
#define WINDMOLEN_MOVE_GENERATION_H_


#include <stddef.h>

#include "move.h"
#include "position.h"



// An upperbound for the maximum number of pseudolegal moves in a chess position.
constexpr size_t MAX_MOVES = 256;


// Generates all legal moves in `position` to `movelist` and returns the number of legal moves found.
size_t generate_legal_moves(struct Position* position, Move movelist[MAX_MOVES]);



#endif /* #ifndef WINDMOLEN_MOVE_GENERATION_H_ */
