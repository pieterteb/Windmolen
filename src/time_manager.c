#include "time_manager.h"

#include <stdint.h>
#include <stdbool.h>

#include "types.h"



void set_time_manager(struct TimeManager* time_manager, Color side_to_move) {
    assert(time_manager != NULL);
    assert(is_valid_color(side_to_move));
    assert(time_manager->black_time > 0 && time_manager->white_time > 0);

    time_manager->white_cutoff_time = get_time_us() + time_manager->white_time / 20 + time_manager->white_increment / 2;
    time_manager->black_cutoff_time = get_time_us() + time_manager->black_time / 20 + time_manager->black_increment / 2;
    time_manager->cutoff_time       = (side_to_move == COLOR_WHITE) ? time_manager->white_cutoff_time
                                                                    : time_manager->black_cutoff_time;
}
