#ifndef BITBOARD_H
#define BITBOARD_H


#include <stdint.h>
#include <stdio.h>

#include "types.h"



#define BITBOARD_EMPTY ((Bitboard)0)

#define FILE_A_BITBOARD ((Bitboard)0x0101010101010101)
#define FILE_B_BITBOARD (FILE_A_BITBOARD << 1)
#define FILE_C_BITBOARD (FILE_A_BITBOARD << 2)
#define FILE_D_BITBOARD (FILE_A_BITBOARD << 3)
#define FILE_E_BITBOARD (FILE_A_BITBOARD << 4)
#define FILE_F_BITBOARD (FILE_A_BITBOARD << 5)
#define FILE_G_BITBOARD (FILE_A_BITBOARD << 6)
#define FILE_H_BITBOARD (FILE_A_BITBOARD << 7)

#define RANK_1_BITBOARD ((Bitboard)0x00000000000000ff)
#define RANK_2_BITBOARD (RANK_1_BITBOARD << 1 * 8)
#define RANK_3_BITBOARD (RANK_1_BITBOARD << 2 * 8)
#define RANK_4_BITBOARD (RANK_1_BITBOARD << 3 * 8)
#define RANK_5_BITBOARD (RANK_1_BITBOARD << 4 * 8)
#define RANK_6_BITBOARD (RANK_1_BITBOARD << 5 * 8)
#define RANK_7_BITBOARD (RANK_1_BITBOARD << 6 * 8)
#define RANK_8_BITBOARD (RANK_1_BITBOARD << 7 * 8)


/**
 * Bitboard with only the given square set.
 */
#define SQUARE_BITBOARD(square) ((Bitboard)1 << (square))


/**
 * True if square is a valid board square (A1–H8).
 */
#define IS_VALID_SQUARE(square) ((square) <= SQUARE_H8)


/**
 * Shifts a bitboard in the given direction, masking wrap-around on files A/H.
 * Supports { N, S, E, W, NE, SE, SW, NW, 2N, 2S }.
 * Returns BITBOARD_EMPTY if direction is invalid.
 */
#define SHIFT_BITBOARD(bitboard, direction) (                                   \
      (direction) == DIRECTION_NORTH     ? (bitboard) << 8                      \
    : (direction) == 2 * DIRECTION_NORTH ? (bitboard) << 16                     \
    : (direction) == DIRECTION_SOUTH     ? (bitboard) >> 8                      \
    : (direction) == 2 * DIRECTION_SOUTH ? (bitboard) >> 16                     \
                                                                                \
    : (direction) == DIRECTION_EAST      ? ((bitboard) & ~FILE_H_BITBOARD) << 1 \
    : (direction) == DIRECTION_WEST      ? ((bitboard) & ~FILE_A_BITBOARD) >> 1 \
                                                                                \
    : (direction) == DIRECTION_NORTHEAST ? ((bitboard) & ~FILE_H_BITBOARD) << 9 \
    : (direction) == DIRECTION_SOUTHEAST ? ((bitboard) & ~FILE_H_BITBOARD) >> 7 \
    : (direction) == DIRECTION_SOUTHWEST ? ((bitboard) & ~FILE_A_BITBOARD) >> 9 \
    : (direction) == DIRECTION_NORTHWEST ? ((bitboard) & ~FILE_A_BITBOARD) << 7 \
                                         : BITBOARD_EMPTY                       \
)


/**
 * Computes pawn attack squares for a bitboard of pawns of given color.
 */
#define PAWN_ATTACKS_BITBOARD(square, color) (                                                                         \
    color == COLOR_WHITE ? SHIFT_BITBOARD(SQUARE_BITBOARD(square), DIRECTION_NORTHEAST) | SHIFT_BITBOARD(SQUARE_BITBOARD(square), DIRECTION_NORTHWEST) \
                         : SHIFT_BITBOARD(SQUARE_BITBOARD(square), DIRECTION_SOUTHEAST) | SHIFT_BITBOARD(SQUARE_BITBOARD(square), DIRECTION_SOUTHWEST) \
)



void print_bitboard(FILE* stream, Bitboard bitboard);

void initialise_bitboards();


struct Magic {
    Bitboard* attack_table;
    Bitboard  mask;
    Bitboard  factor;
    unsigned  shift;
};

unsigned magic_index(PieceType piece_type, Square square, Bitboard occupancy);
extern Bitboard slider_attacks(PieceType piece_type, Square square, Bitboard occupancy);



#endif /* BITBOARD_H */