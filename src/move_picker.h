#ifndef WINDMOLEN_MOVE_PICKER_H_
#define WINDMOLEN_MOVE_PICKER_H_


#include <stddef.h>
#include <stdint.h>

#include "constants.h"
#include "move.h"
#include "position.h"



// Computes the values of the moves in 'move_list' in 'position' for the Most Valuable Victim - Least Valuable Aggressor
// move ordering, and stores them in 'move_values'.
void compute_mvv_lva_values(const struct Position* position, Move move_list[static MAX_MOVES], const size_t move_count,
                            int8_t move_values[static MAX_MOVES]);

// Computes the values of the captures in 'capture_list' in 'position' for the Most Valuable Victim - Least Valuable
// Aggressor capture ordering, and stores them in 'capture_values'.
void compute_capture_mvv_lva_values(const struct Position* position, Move capture_list[static MAX_MOVES],
                                    const size_t capture_count, int8_t capture_values[static MAX_MOVES]);


// Pick the move with the highest 'move_value' from 'move_list' starting from 'start_index'. Using this function ensures
// moves with higher search priority are searched first.
Move pick_move(Move move_list[static MAX_MOVES], int8_t move_values[MAX_MOVES], const size_t move_count,
               const size_t start_index);



#endif /* #ifndef WINDMOLEN_MOVE_PICKER_H_ */
