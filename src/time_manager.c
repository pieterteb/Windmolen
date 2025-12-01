#include "time_manager.h"

#include <stdbool.h>
#include <stdint.h>

#include "piece.h"



void reset_time_manager(struct TimeManager* time_manager) {
    assert(time_manager != NULL);

    *time_manager = (struct TimeManager){0};
}

void update_time_manager(struct TimeManager* time_manager, enum Color side_to_move) {
    assert(time_manager != NULL);
    assert(is_valid_color(side_to_move));
    assert((time_manager->black_time > 0 && time_manager->white_time > 0) || time_manager->move_time > 0);

    const uint64_t time_now = get_time_us();

    if (time_manager->move_time > 0) {
        time_manager->cutoff_time = time_now + time_manager->move_time;
    // } else if (time_manager->moves_to_go > 0) {
    //     time_manager->cutoff_time = time_now
    //                               + ((side_to_move == COLOR_WHITE) ? time_manager->white_time
    //                                                                : time_manager->black_time)
    //                                 / time_manager->moves_to_go;
    } else {
        const uint64_t white_cutoff_time = time_now + time_manager->white_time / 20 + time_manager->white_increment / 2;
        const uint64_t black_cutoff_time = time_now + time_manager->black_time / 20 + time_manager->black_increment / 2;
        time_manager->cutoff_time        = (side_to_move == COLOR_WHITE) ? white_cutoff_time : black_cutoff_time;
    }
}
