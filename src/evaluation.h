#ifndef WINDMOLEN_EVALUATION_H_
#define WINDMOLEN_EVALUATION_H_


#include <limits.h>

#include "position.h"
#include "search.h"



#define DRAWN_VALUE 0
#define MATE_VALUE  (SHRT_MAX / 2)


typedef int Value;


Value evaluate_position(struct Position* position, const struct SearchState* search_state);



#endif /* #ifndef WINDMOLEN_EVALUATION_H_ */
