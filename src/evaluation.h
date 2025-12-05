#ifndef WINDMOLEN_EVALUATION_H_
#define WINDMOLEN_EVALUATION_H_


#include <limits.h>

#include "position.h"



typedef int Score;


constexpr Score DRAW_SCORE = 0;
constexpr Score MATE_SCORE = SHRT_MAX / 2;


void initialize_piece_square_tables();

Score evaluate_position(const struct Position* position);



#endif /* #ifndef WINDMOLEN_EVALUATION_H_ */
