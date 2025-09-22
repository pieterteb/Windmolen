#include <assert.h>
#include <stdlib.h>

#include "move_generation.h"
#include "bitboard.h"
#include "position.h"
#include "types.h"
#include "util.h"



static Move* splat_white_pawn_attacks(Move* movelist, Bitboard pawn_attacks, Direction step) {
    assert(movelist != NULL);

    while (pawn_attacks) {
        Square to = (Square)pop_lsb64(&pawn_attacks);
        *movelist++ = new_move(to - step, to, MOVE_TYPE_CAPTURE);
    }

    return movelist;
}

static Move* pawn_pseudo_attacks(const Position* position, Move* movelist, Color color, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    const Bitboard pawn_bitboard = (color == COLOR_WHITE) ? position->board[PIECE_WHITE_PAWN] : position->board[PIECE_BLACK_PAWN];

    const Direction up_direction = (color == COLOR_WHITE) ? DIRECTION_NORTH : DIRECTION_SOUTH;
    const Direction up_right_direction = up_direction + ((color == COLOR_WHITE) ? DIRECTION_EAST : DIRECTION_WEST);
    const Direction up_left_direction = up_direction + ((color == COLOR_WHITE) ? DIRECTION_WEST : DIRECTION_EAST);

    /* Captures. */
    Bitboard attacks_up_right = shift_bitboard(pawn_bitboard, up_right_direction) & target;
    Bitboard attacks_up_left = shift_bitboard(pawn_bitboard, up_left_direction) & target;
    movelist = splat_white_pawn_attacks(movelist, attacks_up_right, up_right_direction);
    movelist = splat_white_pawn_attacks(movelist, attacks_up_left, up_left_direction);

    if (position->en_passant != BITBOARD_EMPTY) {
        Square en_passant_square = ctz64(position->en_passant);

        if (shift_bitboard(position->en_passant, -up_right_direction) & pawn_bitboard)
            *movelist++ = new_move(en_passant_square, en_passant_square - up_right_direction, MOVE_TYPE_EN_PASSANT_CAPTURE);
        if (shift_bitboard(position->en_passant, -up_left_direction) & pawn_bitboard)
            *movelist++ = new_move(en_passant_square, en_passant_square - up_left_direction, MOVE_TYPE_EN_PASSANT_CAPTURE);
    }

    return movelist;
}
