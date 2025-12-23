#ifndef WINDMOLEN_SCORE_H_
#define WINDMOLEN_SCORE_H_


#include <limits.h>
#include <stdint.h>

#include "board.h"
#include "constants.h"
#include "piece.h"
#include "util.h"



typedef int16_t Score;


static constexpr Score DRAW_SCORE = 0;
static constexpr Score MATE_SCORE = INT16_MAX;
static constexpr Score MAX_SCORE  = MATE_SCORE;
static constexpr Score MIN_SCORE  = -MAX_SCORE;


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


extern const Score piece_square_value_middle_game[PIECE_COUNT][SQUARE_COUNT];
extern const Score piece_square_value_end_game[PIECE_COUNT][SQUARE_COUNT];


// Computes the score belonging to a mate in `ply`.
static INLINE Score mate_score(const size_t ply) {
    assert(ply <= MAX_SEARCH_DEPTH);

    return MATE_SCORE - (Score)ply;
}

// Returns whether `score` is a mate score.
static INLINE bool is_mate_score(const Score score) {
    constexpr Score LONGEST_MATE = MATE_SCORE - MAX_SEARCH_DEPTH;

    return score > LONGEST_MATE || score < -LONGEST_MATE;
}

// Computes the number of plies in which it is mate from `score`, assuming `score` is a mate score.
static INLINE size_t mate_score_to_plies(const Score score) {
    assert(is_mate_score(score));

    if (score < 0)
        return (size_t)(score + MATE_SCORE);

    return (size_t)(MATE_SCORE - score);
}



#endif /* #ifndef WINDMOLEN_SCORE_H_ */
