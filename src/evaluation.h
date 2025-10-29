#ifndef WINDMOLEN_EVALUATION_H_
#define WINDMOLEN_EVALUATION_H_


#include <limits.h>
#include <stddef.h>

#include "position.h"



#define DRAWN_SCORE 0
#define MATE_SCORE  (SHRT_MAX / 2)
#define MAX_SCORE   SHRT_MAX


typedef int Score;


Score evaluate_position(struct Position* position, size_t move_count, size_t current_search_depth);



#endif /* #ifndef WINDMOLEN_EVALUATION_H_ */
