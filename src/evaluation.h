#ifndef WINDMOLEN_EVALUATION_H_
#define WINDMOLEN_EVALUATION_H_


#include <limits.h>

#include "position.h"
#include "score.h"



// Returns the score of `position`.
Score evaluate_position(const struct Position* position);



#endif /* #ifndef WINDMOLEN_EVALUATION_H_ */
