#include "move_picker.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "constants.h"
#include "move.h"
#include "piece.h"
#include "position.h"



// clang-format off

// The value of a capture with `victim_type` and `aggressor_type` that represents what priority it is given in
// the Most Valuable Victim - Least Valuable Aggressor capture ordering, is obtained with
// move_value[victim_type][aggressor_type].
static const int8_t capture_value[PIECE_TYPE_COUNT - 2][PIECE_TYPE_COUNT - 1] = {
    [PIECE_TYPE_PAWN] = {
        [PIECE_TYPE_PAWN] = 6,
        [PIECE_TYPE_KNIGHT] = 5,
        [PIECE_TYPE_BISHOP] = 4,
        [PIECE_TYPE_ROOK] = 3,
        [PIECE_TYPE_QUEEN] = 2,
        [PIECE_TYPE_KING] = 1
    },

    [PIECE_TYPE_KNIGHT] = {
        [PIECE_TYPE_PAWN] = 12,
        [PIECE_TYPE_KNIGHT] = 11,
        [PIECE_TYPE_BISHOP] = 10,
        [PIECE_TYPE_ROOK] = 9,
        [PIECE_TYPE_QUEEN] = 8,
        [PIECE_TYPE_KING] = 7
    },

    [PIECE_TYPE_BISHOP] = {
        [PIECE_TYPE_PAWN] = 18,
        [PIECE_TYPE_KNIGHT] = 17,
        [PIECE_TYPE_BISHOP] = 16,
        [PIECE_TYPE_ROOK] = 15,
        [PIECE_TYPE_QUEEN] = 14,
        [PIECE_TYPE_KING] = 13
    },

    [PIECE_TYPE_ROOK] = {
        [PIECE_TYPE_PAWN] = 24,
        [PIECE_TYPE_KNIGHT] = 23,
        [PIECE_TYPE_BISHOP] = 22,
        [PIECE_TYPE_ROOK] = 21,
        [PIECE_TYPE_QUEEN] = 20,
        [PIECE_TYPE_KING] = 19
    },

    [PIECE_TYPE_QUEEN] = {
        [PIECE_TYPE_PAWN] = 30,
        [PIECE_TYPE_KNIGHT] = 29,
        [PIECE_TYPE_BISHOP] = 28,
        [PIECE_TYPE_ROOK] = 27,
        [PIECE_TYPE_QUEEN] = 26,
        [PIECE_TYPE_KING] = 25
    }
};
// clang-format on


void compute_mvv_lva_values(const struct Position* position, Move move_list[static MAX_MOVES], const size_t move_count,
                            int8_t move_values[static MAX_MOVES]) {
    assert(position != nullptr);
    assert(move_list != nullptr);
    assert(move_values != nullptr);

    // We assign each move a value. The higher the value, the higher the priority it gets when picking a move.

    const int8_t NON_CAPTURE_VALUE = 0;

    for (size_t i = 0; i < move_count; ++i) {
        const enum PieceType aggressor_type = type_of_piece(piece_on_square(position, move_source(move_list[i])));
        const enum Piece victim             = piece_on_square(position, move_destination(move_list[i]));

        if (type_of_move(move_list[i]) == MOVE_TYPE_EN_PASSANT) {
            move_values[i] = capture_value[PIECE_TYPE_PAWN][PIECE_TYPE_PAWN];
        } else {
            move_values[i] = (victim == PIECE_NONE) ? NON_CAPTURE_VALUE
                                                    : capture_value[type_of_piece(victim)][aggressor_type];
        }
    }
}

void compute_capture_mvv_lva_values(const struct Position* position, Move capture_list[static MAX_MOVES],
                                    const size_t capture_count, int8_t capture_values[static MAX_MOVES]) {
    assert(position != nullptr);
    assert(capture_list != nullptr);
    assert(capture_values != nullptr);

    // We assign each capture a value. The higher the value, the higher the priority it gets when picking a move. In
    // this function, it is assumed that all moves are captures.

    for (size_t i = 0; i < capture_count; ++i) {
        if (type_of_move(capture_list[i]) == MOVE_TYPE_EN_PASSANT) {
            capture_values[i] = capture_value[PIECE_TYPE_PAWN][PIECE_TYPE_PAWN];
        } else {
            const enum PieceType victim_type = type_of_piece(
            piece_on_square(position, move_destination(capture_list[i])));

            const enum PieceType aggressor_type = type_of_piece(
            piece_on_square(position, move_source(capture_list[i])));

            capture_values[i] = capture_value[victim_type][aggressor_type];
        }
    }
}


Move pick_move(Move move_list[static MAX_MOVES], int8_t move_values[MAX_MOVES], const size_t move_count,
               const size_t start_index) {
    assert(move_list != nullptr);
    assert(move_values != nullptr);
    assert(move_count > 0);
    assert(start_index < move_count);

    // To pick a move, we search for the move with the highest value, i.e. the highest priority. This is the move we
    // will search next. If the move at the start index is not the best move, we need to swap it with the best move.
    // However, since in the next pick_move() call, start_index will be 1 greater than in the current call, it is not
    // necessary to update the move located at start_index, as we will never search it again. Therefore, we only update
    // the move at best_index.

    size_t best_index = start_index;

    for (size_t i = start_index; i < move_count; ++i)
        if (move_values[i] > move_values[best_index])
            best_index = i;

    // Retrieve best move and place the move that is located at the start index further in the move list such that it
    // can be considered in a future pick_move() call.
    const Move best_move = move_list[best_index];

    if (best_index != start_index) {
        move_list[best_index]   = move_list[start_index];
        move_values[best_index] = move_values[start_index];
    }

    return best_move;
}
