#ifndef WINDMOLEN_MOVE_PICKER_H_
#define WINDMOLEN_MOVE_PICKER_H_


#include <stddef.h>

#include "constants.h"
#include "move.h"
#include "position.h"



void mvv_lva_sort(Move move_list[static MAX_MOVES], const size_t move_count, const struct Position* position);



#endif /* #ifndef WINDMOLEN_MOVE_PICKER_H_ */
