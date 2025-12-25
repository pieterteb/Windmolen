#ifndef WINDMOLEN_SCORE_H_
#define WINDMOLEN_SCORE_H_


#include <limits.h>
#include <stdint.h>

#include "board.h"
#include "constants.h"
#include "piece.h"
#include "util.h"



typedef int16_t Score;  // The Score type is used to store and interpret scores.
typedef int Value;      // The Value type is used for calculation of scores.


static constexpr Score DRAW_SCORE = 0;
static constexpr Score MATE_SCORE = INT16_MAX;
static constexpr Score MAX_SCORE  = MATE_SCORE;
static constexpr Score MIN_SCORE  = -MAX_SCORE;

static constexpr Value DRAW_VALUE = DRAW_SCORE;
static constexpr Value MATE_VALUE = MATE_SCORE;
static constexpr Value MAX_VALUE  = MAX_SCORE;
static constexpr Value MIN_VALUE  = MIN_SCORE;


// clang-format off
static constexpr int game_phase_increment[PIECE_TYPE_COUNT] = {
    [PIECE_TYPE_PAWN]   = 0,
    [PIECE_TYPE_KNIGHT] = 1,
    [PIECE_TYPE_BISHOP] = 1,
    [PIECE_TYPE_ROOK]   = 2,
    [PIECE_TYPE_QUEEN]  = 4,
    [PIECE_TYPE_KING]   = 0,
};
// clang-format on


extern const Value piece_square_value_middle_game[PIECE_COUNT][SQUARE_COUNT];
extern const Value piece_square_value_end_game[PIECE_COUNT][SQUARE_COUNT];


// Returns whether `score` is valid.
static INLINE bool is_valid_score(const Score score) {
    // score <= MAX_SCORE always holds in this case.
    static_assert(MAX_SCORE == INT16_MAX);

    return score >= MIN_SCORE;
}

// Returns whether `value` is valid.
static INLINE bool is_valid_value(const Value value) {
    return value >= MIN_VALUE && value <= MAX_VALUE;
}


// Computes the value belonging to a mate in `ply`.
static INLINE Value mate_value(const size_t ply) {
    assert(ply <= MAX_SEARCH_DEPTH);

    return MATE_VALUE - (Value)ply;
}

// Returns whether `value` is a mate value.
static INLINE bool is_mate_value(const Value value) {
    assert(is_valid_value(value));

    constexpr Value LONGEST_MATE = MATE_VALUE - MAX_SEARCH_DEPTH;

    return value >= LONGEST_MATE || value <= -LONGEST_MATE;
}

// Computes the number of plies in which it is mate from `value` and returns that as a value, assuming `value` is a mate
// score.
static INLINE Value mate_score_in_plies(const Value value) {
    assert(is_valid_value(value));
    assert(is_mate_value(value));

    if (value < 0)
        return -MATE_VALUE - value;

    return MATE_VALUE - value;
}



#endif /* #ifndef WINDMOLEN_SCORE_H_ */
