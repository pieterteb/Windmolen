#include "evaluation.h"

#include <assert.h>

#include "piece.h"
#include "position.h"
#include "score.h"



Score evaluate_position(const struct Position* position) {
    assert(position != nullptr);

    const struct PositionInfo* position_info = position->info;

    const Score middle_game_score = position_info->middle_game_score[COLOR_WHITE]
                                  - position_info->middle_game_score[COLOR_BLACK];
    const Score end_game_score = position_info->end_game_score[COLOR_WHITE]
                               - position_info->end_game_score[COLOR_BLACK];

    /* Tapered eval. */
    int middle_game_phase = position_info->game_phase;
    if (middle_game_phase > 24)
        middle_game_phase = 24;  // In case of an early promotion.
    const int end_game_phase = 24 - middle_game_phase;

    return (Score)(middle_game_score * middle_game_phase + end_game_score * end_game_phase) / 24;
}
