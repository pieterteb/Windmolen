#include "evaluation.h"

#include <assert.h>
#include <stddef.h>

#include "position.h"
#include "types.h"
#include "util.h"



// -2 because white and black pawns are the same, and the king has no value.
static const Score piece_values[PIECE_TYPE_COUNT - 2] = {100, 350, 350, 525, 1000};


Score evaluate_position(struct Position* position) {
    assert(position != NULL);

    Score score = DRAWN_SCORE;

    score += piece_values[PIECE_TYPE_PAWN]
           * (popcount64(piece_occupancy_by_piece(position, PIECE_WHITE_PAWN))
              - popcount64(piece_occupancy_by_piece(position, PIECE_BLACK_PAWN)));
    score += piece_values[PIECE_TYPE_KNIGHT]
           * (popcount64(piece_occupancy_by_piece(position, PIECE_WHITE_KNIGHT))
              - popcount64(piece_occupancy_by_piece(position, PIECE_BLACK_KNIGHT)));
    score += piece_values[PIECE_TYPE_BISHOP]
           * (popcount64(piece_occupancy_by_piece(position, PIECE_WHITE_BISHOP))
              - popcount64(piece_occupancy_by_piece(position, PIECE_BLACK_BISHOP)));
    score += piece_values[PIECE_TYPE_ROOK]
           * (popcount64(piece_occupancy_by_piece(position, PIECE_WHITE_ROOK))
              - popcount64(piece_occupancy_by_piece(position, PIECE_BLACK_ROOK)));
    score += piece_values[PIECE_TYPE_QUEEN]
           * (popcount64(piece_occupancy_by_piece(position, PIECE_WHITE_QUEEN))
              - popcount64(piece_occupancy_by_piece(position, PIECE_BLACK_QUEEN)));

    return score;
}
