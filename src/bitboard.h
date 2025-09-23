#ifndef BITBOARD_H
#define BITBOARD_H


#include "types.h"



#define BISHOP_ENTRY_COUNT 5248
#define ROOK_ENTRY_COUNT 102400

Bitboard piece_base_attack_table[PIECE_TYPE_COUNT][SQUARE_COUNT];
extern Bitboard slider_attack_table[BISHOP_ENTRY_COUNT + ROOK_ENTRY_COUNT];


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



/* Several macro functions for static evaluation. */


// Static evaluation version of file_from_square().
//
// File of square.
#define FILE_FROM_SQUARE(square) ((File)((square) & 7))

// Static evaluation version of rank_from_square().
//
// Rank of square.
#define RANK_FROM_SQUARE(square) ((Rank)((square) >> 3))

// Static evaluation version of square_bitboard().
//
// Bitboard with only the given square set.
#define SQUARE_BITBOARD(square) ((Bitboard)1 << (square))

// Static evaluation version of file_bitboard().
//
// Bitboard with all squares on file set.
#define FILE_BITBOARD(file) (FILE_A_BITBOARD << (file))

// Static evaluation version of rank_bitboard().
//
// Bitboard with all squares on rank set.
#define RANK_BITBOARD(rank) (RANK_1_BITBOARD << ((rank) * 8))

// Static evaluation version of coordinates_bitboard().
//
// Bitboard with only the square specified by file and rank set.
#define COORDINATES_BITBOARD(file, rank) (FILE_BITBOARD(file) & RANK_BITBOARD(rank))

// Static evaluation version of shift_bitboard().
//
// Shifts a bitboard in the given direction, masking wrap-around on files A/H.
// Supports { N, S, E, W, NE, SE, SW, NW, 2N, 2S }.
// Returns BITBOARD_EMPTY if direction is invalid.
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

// Static evaluation version of pawn_attacks_bitboard().
//
// Computes pawn attack squares for a bitboard of pawns of given color.
#define PAWN_ATTACKS_BITBOARD(bitboard, color) (                                                                             \
    color == COLOR_WHITE ? SHIFT_BITBOARD((bitboard), DIRECTION_NORTHEAST) | SHIFT_BITBOARD((bitboard), DIRECTION_NORTHWEST) \
                         : SHIFT_BITBOARD((bitboard), DIRECTION_SOUTHEAST) | SHIFT_BITBOARD((bitboard), DIRECTION_SOUTHWEST) \
)



// File of square.
static inline File file_from_square(Square square) {
    assert(is_valid_square(square));

    return (File)(square & 7); // Fast modulo 8.
}

// Rank of square.
static inline Rank rank_from_square(Square square) {
    assert(is_valid_square(square));

    return (Rank)(square >> 3); // Fast division by 8.
}

// Bitboard with only the given square set.
static inline Bitboard square_bitboard(Square square) {
    assert(is_valid_square(square));

    return (Bitboard)1 << square;
}

// Bitboard with all squares on file set.
static inline Bitboard file_bitboard(File file) {
    assert(is_valid_file(file));

    return FILE_A_BITBOARD << file;
}

// Bitboard with all squares on the same file as square set.
static inline Bitboard file_bitboard_from_square(Square square) {
    assert(is_valid_square(square));

    return file_bitboard(file_from_square(square));
}

// Bitboard with all squares on rank set.
static inline Bitboard rank_bitboard(Rank rank) {
    assert(is_valid_rank(rank));

    return RANK_1_BITBOARD << (rank * 8);
}

// Bitboard with all squares on the same rank as square set.
static inline Bitboard rank_bitboard_from_square(Square square) {
    assert(is_valid_square(square));

    return rank_bitboard(rank_from_square(square));
}

// Bitboard with only the square specified by file and rank set.
static inline Bitboard coordinates_bitboard(File file, Rank rank) {
    assert(is_valid_file(file) && is_valid_rank(rank));

    return file_bitboard(file) & rank_bitboard(rank);
}

// Shifts a bitboard in the given direction, masking wrap-around on files A/H.
// Supports { N, S, E, W, NE, SE, SW, NW, 2N, 2S }.
// Returns BITBOARD_EMPTY if direction is invalid.
static inline Bitboard shift_bitboard(Bitboard bitboard, Direction direction) {
    return (direction == DIRECTION_NORTH)     ? (bitboard) << 8
         : (direction == 2 * DIRECTION_NORTH) ? (bitboard) << 16
         : (direction == DIRECTION_SOUTH)     ? (bitboard) >> 8
         : (direction == 2 * DIRECTION_SOUTH) ? (bitboard) >> 16

         : (direction == DIRECTION_EAST)      ? (bitboard & ~FILE_H_BITBOARD) << 1
         : (direction == DIRECTION_WEST)      ? (bitboard & ~FILE_A_BITBOARD) >> 1

         : (direction == DIRECTION_NORTHEAST) ? (bitboard & ~FILE_H_BITBOARD) << 9
         : (direction == DIRECTION_SOUTHEAST) ? (bitboard & ~FILE_H_BITBOARD) >> 7
         : (direction == DIRECTION_SOUTHWEST) ? (bitboard & ~FILE_A_BITBOARD) >> 9
         : (direction == DIRECTION_NORTHWEST) ? (bitboard & ~FILE_A_BITBOARD) << 7
                                              : BITBOARD_EMPTY;
}

// Computes pawn attack squares for a bitboard of pawns of given color.
static inline Bitboard pawn_attacks_bitboard(Bitboard bitboard, Color color) {
    assert(is_valid_color(color));

    // We use static evaluation here because directions are known beforehand.
    return (color == COLOR_WHITE) ? SHIFT_BITBOARD(bitboard, DIRECTION_NORTHEAST) | SHIFT_BITBOARD(bitboard, DIRECTION_NORTHWEST)
                                  : SHIFT_BITBOARD(bitboard, DIRECTION_SOUTHEAST) | SHIFT_BITBOARD(bitboard, DIRECTION_SOUTHWEST);
}



void initialise_bitboards();

struct Magic {
    Bitboard* attack_table;
    Bitboard  mask;
    Bitboard  factor;
    unsigned  shift;
};

unsigned magic_index(PieceType piece_type, Square square, Bitboard occupancy);
extern Bitboard slider_attacks(PieceType piece_type, Square square, Bitboard occupancy);


// Returns a chessboard representation of the given bitboard.
// size_out can be ignored by passing NULL.
char* bitboard_to_string(Bitboard bitboard, size_t* size_out);



#endif /* BITBOARD_H */