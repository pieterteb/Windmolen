#include "time_manager.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "piece.h"



void reset_time_manager(struct TimeManager* time_manager) {
    assert(time_manager != nullptr);

    // Zero everything except for move overhead, as that is determined by the engine, not the search parameters.
    memset(time_manager, 0, offsetof(struct TimeManager, move_overhead));
}

void update_time_manager(struct TimeManager* time_manager, const enum Color side_to_move) {
    assert(time_manager != nullptr);
    assert(is_valid_color(side_to_move));
    assert((time_manager->black_time > 0 && time_manager->white_time > 0) || time_manager->move_time > 0);

    uint64_t search_time;
    if (time_manager->move_time != 0) {
        search_time = time_manager->move_time;
    } else {
        const uint64_t white_search_time = time_manager->white_time / 20 + time_manager->white_increment / 2;
        const uint64_t black_search_time = time_manager->black_time / 20 + time_manager->black_increment / 2;
        search_time                      = (side_to_move == COLOR_WHITE) ? white_search_time : black_search_time;
    }

    search_time = (time_manager->move_overhead > search_time) ? 0 : search_time - time_manager->move_overhead;
    time_manager->cutoff_time = get_time_us() + search_time;
}
