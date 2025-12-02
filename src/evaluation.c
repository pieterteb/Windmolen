#include "evaluation.h"

#include <assert.h>

#include "position.h"
#include "util.h"



// -2 because white and black pawns are the same, and the king has no value.
static const Score piece_values[PIECE_TYPE_COUNT - 2] = {100, 350, 350, 525, 1000};


Score evaluate_position(const struct Position* position) {
    assert(position != nullptr);

    Score score = DRAW_SCORE;

    score += piece_values[PIECE_TYPE_PAWN]
           * (popcount64(piece_occupancy(position, COLOR_WHITE, PIECE_TYPE_PAWN))
              - popcount64(piece_occupancy(position, COLOR_BLACK, PIECE_TYPE_PAWN)));
    score += piece_values[PIECE_TYPE_KNIGHT]
           * (popcount64(piece_occupancy(position, COLOR_WHITE, PIECE_TYPE_KNIGHT))
              - popcount64(piece_occupancy(position, COLOR_BLACK, PIECE_TYPE_KNIGHT)));
    score += piece_values[PIECE_TYPE_BISHOP]
           * (popcount64(piece_occupancy(position, COLOR_WHITE, PIECE_TYPE_BISHOP))
              - popcount64(piece_occupancy(position, COLOR_BLACK, PIECE_TYPE_BISHOP)));
    score += piece_values[PIECE_TYPE_ROOK]
           * (popcount64(piece_occupancy(position, COLOR_WHITE, PIECE_TYPE_ROOK))
              - popcount64(piece_occupancy(position, COLOR_BLACK, PIECE_TYPE_ROOK)));
    score += piece_values[PIECE_TYPE_QUEEN]
           * (popcount64(piece_occupancy(position, COLOR_WHITE, PIECE_TYPE_QUEEN))
              - popcount64(piece_occupancy(position, COLOR_BLACK, PIECE_TYPE_QUEEN)));

    return score;
}
