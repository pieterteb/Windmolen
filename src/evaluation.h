#ifndef WINDMOLEN_EVALUATION_H_
#define WINDMOLEN_EVALUATION_H_


#include <limits.h>
#include <stddef.h>

#include "position.h"



#define DRAWN_SCORE ((Score)0)
#define MATE_SCORE  ((Score)(SHRT_MAX / 2))


typedef int Score;


Score evaluate_position(struct Position* position);



#endif /* #ifndef WINDMOLEN_EVALUATION_H_ */
