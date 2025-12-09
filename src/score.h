#ifndef WINDMOLEN_SCORE_H_
#define WINDMOLEN_SCORE_H_


#include <limits.h>

#include "board.h"
#include "piece.h"



typedef int Score;


static constexpr Score DRAW_SCORE = 0;
static constexpr Score MATE_SCORE = INT_MAX / 2;
static constexpr Score MAX_SCORE  = MATE_SCORE;
static constexpr Score MIN_SCORE  = -MAX_SCORE;


// clang-format off
static constexpr int game_phase_increment[PIECE_TYPE_COUNT] = {
    [PIECE_TYPE_PAWN] = 0,
    [PIECE_TYPE_KNIGHT] = 1,
    [PIECE_TYPE_BISHOP] = 1,
    [PIECE_TYPE_ROOK] = 2,
    [PIECE_TYPE_QUEEN] = 4,
    [PIECE_TYPE_KING] = 0,
};
// clang-format on


extern const Score piece_square_middle_game[PIECE_COUNT][SQUARE_COUNT];
extern const Score piece_square_end_game[PIECE_COUNT][SQUARE_COUNT];



#endif /* #ifndef WINDMOLEN_SCORE_H_ */
