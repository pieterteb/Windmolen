#ifndef WINDMOLEN_EVALUATION_H_
#define WINDMOLEN_EVALUATION_H_


#include <limits.h>

#include "position.h"



typedef int Score;


constexpr Score DRAW_SCORE = 0;
constexpr Score MATE_SCORE = SHRT_MAX / 2;


// Initializes the piece square tables used for evaluating a position.
void initialize_piece_square_tables();

// Evaluates `position` and returns its score.
Score evaluate_position(const struct Position* position);



#endif /* #ifndef WINDMOLEN_EVALUATION_H_ */
