#ifndef WINDMOLEN_MOVE_GENERATION_H_
#define WINDMOLEN_MOVE_GENERATION_H_


#include <stddef.h>

#include "constants.h"
#include "move.h"
#include "position.h"



// Generates all legal moves in `position` to `movelist` and returns the number of legal moves found.
size_t generate_legal_moves(const struct Position* position, Move movelist[MAX_MOVES]);



#endif /* #ifndef WINDMOLEN_MOVE_GENERATION_H_ */
