#ifndef BITBOARD_H
#define BITBOARD_H


#include <assert.h>

#include "types.h"



#define BISHOP_ENTRY_COUNT 5248
#define ROOK_ENTRY_COUNT 102400


Bitboard piece_base_attack(PieceType piece_type, Square square);

#define EMPTY_BITBOARD  ((Bitboard)0)

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


extern Bitboard piece_base_attack_table[PIECE_TYPE_COUNT][SQUARE_COUNT];
extern Bitboard slider_attack_table[BISHOP_ENTRY_COUNT + ROOK_ENTRY_COUNT];

extern Bitboard diagonals[15]; // Indices: (rank - file) + 7   0-14
extern Bitboard antidiagonals[15]; // Indices: (rank + file)   0-14
extern Bitboard line_bitboards[SQUARE_COUNT][SQUARE_COUNT];


static inline Bitboard diagonal_bitboard(int diagonal_index) {
    assert(diagonal_index >= -7 && diagonal_index <= 7);

    return diagonals[diagonal_index + 7];
}

static inline Bitboard antidiagonal_bitboard(int antidiagonal_index) {
    assert(antidiagonal_index >= 0 && antidiagonal_index <= 14);

    return antidiagonals[antidiagonal_index];
}

static inline Bitboard line_bitboard(Square square1, Square square2) {
    assert(is_valid_square(square1));
    assert(is_valid_square(square2));

    return line_bitboards[square1][square2];
}


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

// Square with given coordinates.
static inline Square coordinates_square(File file, Rank rank) {
    assert(is_valid_file(file) && is_valid_rank(rank));

    return (Square)(file * DIRECTION_EAST + rank * DIRECTION_NORTH);
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
                                              : EMPTY_BITBOARD;
}

// Computes pawn attack squares for a bitboard of pawns of given color.
static inline Bitboard pawn_attacks_bitboard(Bitboard bitboard, Color color) {
    assert(is_valid_color(color));

    // We use static evaluation here because directions are known beforehand.
    return (color == COLOR_WHITE) ? shift_bitboard(bitboard, DIRECTION_NORTHEAST) | shift_bitboard(bitboard, DIRECTION_NORTHWEST)
                                  : shift_bitboard(bitboard, DIRECTION_SOUTHEAST) | shift_bitboard(bitboard, DIRECTION_SOUTHWEST);
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