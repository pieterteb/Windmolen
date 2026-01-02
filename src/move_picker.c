#include "move_picker.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "constants.h"
#include "move.h"
#include "piece.h"
#include "position.h"



void mvv_lva_sort(Move move_list[static MAX_MOVES], const size_t move_count, const struct Position* position) {
    assert(move_list != nullptr);
    assert(position != nullptr);

    // We assign each move an integer that represents what priority it is given in the Most Valuable Victim - Least
    // Valuable Aggressor capture ordering. The lower the value, the higher the priority. The value of a capture is
    // computed by aggressor_value + victim_value. A non-capture will get a value of NON_CAPTURE. Finally, we use a
    // counting sort algorithm to sort the captures.

    if (move_count == 0)
        return;

    // clang-format off
    static const int aggressor_value[] = {
        [PIECE_TYPE_PAWN]   = 0,
        [PIECE_TYPE_KNIGHT] = 1,
        [PIECE_TYPE_BISHOP] = 2,
        [PIECE_TYPE_ROOK]   = 3,
        [PIECE_TYPE_QUEEN]  = 4,
        [PIECE_TYPE_KING]   = 5,
    };
    static const int victim_value[] = {
        [PIECE_TYPE_PAWN]   = (PIECE_TYPE_COUNT - 1) * 4,
        [PIECE_TYPE_KNIGHT] = (PIECE_TYPE_COUNT - 1) * 3,
        [PIECE_TYPE_BISHOP] = (PIECE_TYPE_COUNT - 1) * 2,
        [PIECE_TYPE_ROOK]   = (PIECE_TYPE_COUNT - 1) * 1,
        [PIECE_TYPE_QUEEN]  = (PIECE_TYPE_COUNT - 1) * 0,
    };
    // clang-format on

    assert(aggressor_value[PIECE_TYPE_KING] + victim_value[PIECE_TYPE_PAWN] == 29);
    constexpr int NON_CAPTURE_VALUE = 30;  // victim_value[PIECE_TYPE_NONE]

    int capture_values[MAX_MOVES];
    size_t capture_value_count[NON_CAPTURE_VALUE + 1] = {0};

    for (size_t i = 0; i < move_count; ++i) {
        const enum PieceType aggressor_type = type_of_piece(piece_on_square(position, move_source(move_list[i])));
        const enum Piece victim             = piece_on_square(position, move_destination(move_list[i]));

        if (type_of_move(move_list[i]) == MOVE_TYPE_EN_PASSANT) {
            capture_values[i] = aggressor_value[PIECE_TYPE_PAWN] + victim_value[PIECE_TYPE_PAWN];
        } else if (victim == PIECE_NONE) {
            capture_values[i] = 30;
        } else {
            const enum PieceType victim_type = type_of_piece(victim);
            capture_values[i]                = aggressor_value[aggressor_type] + victim_value[victim_type];
        }

        ++capture_value_count[capture_values[i]];
    }

    // Compute the index at which a certain value starts.
    for (size_t i = 1; i <= NON_CAPTURE_VALUE; ++i)
        capture_value_count[i] += capture_value_count[i - 1];

    Move temp_move_list[MAX_MOVES];
    for (int i = (int)move_count - 1; i >= 0; --i) {
        const int value                              = capture_values[i];
        temp_move_list[--capture_value_count[value]] = move_list[i];
    }

    memcpy(move_list, temp_move_list, move_count * sizeof(*move_list));
}
