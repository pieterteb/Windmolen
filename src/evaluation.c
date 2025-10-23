#include "evaluation.h"

#include <assert.h>
#include <stddef.h>

#include "move_generation.h"
#include "position.h"
#include "search.h"
#include "types.h"
#include "util.h"



// -2 because white and black pawns are the same, and the king has no value.
static const Value piece_values[PIECE_TYPE_COUNT - 2] = {100, 350, 350, 525, 1000};


Value evaluate_position(struct Position* position, const struct SearchState* search_state) {
    assert(position != NULL);
    assert(search_state != NULL);

    Value value = DRAWN_VALUE;

    Move movelist[MAX_MOVES];
    size_t move_count = generate_legal_moves(position, movelist);

    if (move_count == 0) {
        // If there are no moves, it is either checkmate or stalemate. We want to checkmate as fast as possible, and
        // prolong a draw for as long as possible.
        if (position->checkers[position->side_to_move] != EMPTY_BITBOARD)
            value = MATE_VALUE - search_state->current_search_depth;
        else
            value -= search_state->current_search_depth;
    } else {
        value += piece_values[PIECE_TYPE_PAWN]
               * (popcount64(piece_occupancy_by_piece(position, PIECE_WHITE_PAWN))
                  - popcount64(piece_occupancy_by_piece(position, PIECE_BLACK_PAWN)));
        value += piece_values[PIECE_TYPE_KNIGHT]
               * (popcount64(piece_occupancy_by_piece(position, PIECE_WHITE_KNIGHT))
                  - popcount64(piece_occupancy_by_piece(position, PIECE_BLACK_KNIGHT)));
        value += piece_values[PIECE_TYPE_BISHOP]
               * (popcount64(piece_occupancy_by_piece(position, PIECE_WHITE_BISHOP))
                  - popcount64(piece_occupancy_by_piece(position, PIECE_BLACK_BISHOP)));
        value += piece_values[PIECE_TYPE_ROOK]
               * (popcount64(piece_occupancy_by_piece(position, PIECE_WHITE_ROOK))
                  - popcount64(piece_occupancy_by_piece(position, PIECE_BLACK_ROOK)));
        value += piece_values[PIECE_TYPE_QUEEN]
               * (popcount64(piece_occupancy_by_piece(position, PIECE_WHITE_QUEEN))
                  - popcount64(piece_occupancy_by_piece(position, PIECE_BLACK_QUEEN)));
    }

    return (search_state->side_to_evaluate == COLOR_WHITE) ? value : -value;
}
