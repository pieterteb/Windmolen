#ifndef BITBOARD_H
#define BITBOARD_H


#include <assert.h>
#include <stddef.h>

#include "types.h"



#define BISHOP_ENTRY_COUNT 5248
#define ROOK_ENTRY_COUNT   102400


#define EMPTY_BITBOARD ((Bitboard)0)


extern Bitboard piece_base_attack_table[PIECE_TYPE_COUNT][SQUARE_COUNT];

// clang-format off
static const Bitboard file_bitboards[FILE_COUNT] = {
    (Bitboard)0x0101010101010101 << 0,
    (Bitboard)0x0101010101010101 << 1,
    (Bitboard)0x0101010101010101 << 2,
    (Bitboard)0x0101010101010101 << 3,
    (Bitboard)0x0101010101010101 << 4,
    (Bitboard)0x0101010101010101 << 5,
    (Bitboard)0x0101010101010101 << 6,
    (Bitboard)0x0101010101010101 << 7,
};
static const Bitboard rank_bitboards[RANK_COUNT] = {
    (Bitboard)0x00000000000000ff << 0 * 8,
    (Bitboard)0x00000000000000ff << 1 * 8,
    (Bitboard)0x00000000000000ff << 2 * 8,
    (Bitboard)0x00000000000000ff << 3 * 8,
    (Bitboard)0x00000000000000ff << 4 * 8,
    (Bitboard)0x00000000000000ff << 5 * 8,
    (Bitboard)0x00000000000000ff << 6 * 8,
    (Bitboard)0x00000000000000ff << 7 * 8,
};
// clang-format on

extern Bitboard diagonals[15];     // Indices: (rank - file) + 7   0-14
extern Bitboard antidiagonals[15]; // Indices: (rank + file)   0-14
extern Bitboard line_bitboards[SQUARE_COUNT][SQUARE_COUNT];
extern Bitboard between_bitboards[SQUARE_COUNT][SQUARE_COUNT];

struct Magic {
    Bitboard* attack_table;
    Bitboard mask;
    Bitboard factor;
    unsigned shift;
};

extern struct Magic bishop_magic_table[SQUARE_COUNT];
extern struct Magic rook_magic_table[SQUARE_COUNT];


/* Returns a bitboard of the diagonal with index `diagonal_index`. */
static inline Bitboard diagonal_bitboard(int diagonal_index) {
    assert(diagonal_index >= -7 && diagonal_index <= 7);

    return diagonals[diagonal_index + 7];
}

/* Returns a bitboard of the anti-diagonal with index `antidiagonal_index`. */
static inline Bitboard antidiagonal_bitboard(int antidiagonal_index) {
    assert(antidiagonal_index >= 0 && antidiagonal_index <= 14);

    return antidiagonals[antidiagonal_index];
}

/* Returns a bitboard of the line that connects `square1` and `square2` or `EMPTY_BITBOARD` if no such line exists. */
static inline Bitboard line_bitboard(Square square1, Square square2) {
    assert(is_valid_square(square1));
    assert(is_valid_square(square2));

    return line_bitboards[square1][square2];
}

/* Returns a bitboard of the squares that lie between `square1` and `square2` including `square2` but excluding
 * `square1`. If no such line exists, only `square2` is returned. For example, if `square1 == SQUARE_A1` and `square2 ==
 * SQUARE_D4`, then a bitboard with the squares B2, C3 and D4 is returned. If `square1 == SQUARE_A1` and `square2 ==
 * SQUARE_C2`, a bitboard with only C2 is returned. */
static inline Bitboard between_bitboard(Square square1, Square square2) {
    assert(is_valid_square(square1));
    assert(is_valid_square(square2));

    return between_bitboards[square1][square2];
}


/* Returns the file that `square` lies on. */
static inline File file_from_square(Square square) {
    assert(is_valid_square(square));

    return (File)(square & 7); // Fast modulo 8.
}

/* Returns the rank that `square` lies on. */
static inline Rank rank_from_square(Square square) {
    assert(is_valid_square(square));

    return (Rank)(square >> 3); // Fast division by 8.
}

/* Returns a bitboard of `square`. */
static inline Bitboard square_bitboard(Square square) {
    assert(is_valid_square(square));

    return (Bitboard)1 << square;
}

/* Returns a bitboard of `square`. Static evaluation version of `square_bitboard()`. */
#define SQUARE_BITBOARD(square) ((Bitboard)1 << square)

/* Returns a bitboard of `file`. */
static inline Bitboard file_bitboard(File file) {
    assert(is_valid_file(file));

    return file_bitboards[file];
}

/* Returns a bitboard of `rank`. */
static inline Bitboard rank_bitboard(Rank rank) {
    assert(is_valid_rank(rank));

    return rank_bitboards[rank];
}

/* Returns a bitboard of the file of `square`. */
static inline Bitboard file_bitboard_from_square(Square square) {
    assert(is_valid_square(square));

    return file_bitboards[file_from_square(square)];
}

/* Returns a bitboard of the rank of `square`. */
static inline Bitboard rank_bitboard_from_square(Square square) {
    assert(is_valid_square(square));

    return rank_bitboards[rank_from_square(square)];
}

/* Returns a bitboard of the square described by `file` and `rank`. */
static inline Bitboard coordinate_bitboard(File file, Rank rank) {
    assert(is_valid_file(file) && is_valid_rank(rank));

    return file_bitboard(file) & rank_bitboard(rank);
}

/* Returns the square described by `file` and `rank`. */
static inline Square coordinate_square(File file, Rank rank) {
    assert(is_valid_file(file) && is_valid_rank(rank));

    // Should compile to a single LEA instruction on modern CPUs.
    return (Square)(file * DIRECTION_EAST + rank * DIRECTION_NORTH);
}

/* Shifts `bitboard` in direction `direction`, masking wrap-around on the A/H files. Supports { N, S, E, W, NE, SE, SW,
 * NW, 2N, 2S }. Returns `EMPTY_BITBOARD` if direction is invalid. Only use this function if `direction` is a
 * compile-time constant. */
static inline Bitboard shift_bitboard(Bitboard bitboard, Direction direction) {
    return (direction == DIRECTION_NORTH)     ? (bitboard) << 8
         : (direction == 2 * DIRECTION_NORTH) ? (bitboard) << 16
         : (direction == DIRECTION_SOUTH)     ? (bitboard) >> 8
         : (direction == 2 * DIRECTION_SOUTH) ? (bitboard) >> 16

         : (direction == DIRECTION_EAST) ? (bitboard & ~file_bitboard(FILE_H)) << 1
         : (direction == DIRECTION_WEST) ? (bitboard & ~file_bitboard(FILE_A)) >> 1

         : (direction == DIRECTION_NORTHEAST) ? (bitboard & ~file_bitboard(FILE_H)) << 9
         : (direction == DIRECTION_SOUTHEAST) ? (bitboard & ~file_bitboard(FILE_H)) >> 7
         : (direction == DIRECTION_SOUTHWEST) ? (bitboard & ~file_bitboard(FILE_A)) >> 9
         : (direction == DIRECTION_NORTHWEST) ? (bitboard & ~file_bitboard(FILE_A)) << 7
                                              : EMPTY_BITBOARD;
}

/* Returns bitboard of attack squares for `bitboard` of white pawns. */
static inline Bitboard white_pawn_attacks_bitboard(Bitboard bitboard) {
    return shift_bitboard(bitboard, DIRECTION_NORTHEAST) | shift_bitboard(bitboard, DIRECTION_NORTHWEST);
}

/* Returns bitboard of attack squares for `bitboard` of black pawns. */
static inline Bitboard black_pawn_attacks_bitboard(Bitboard bitboard) {
    return shift_bitboard(bitboard, DIRECTION_SOUTHEAST) | shift_bitboard(bitboard, DIRECTION_SOUTHWEST);
}

/* Returns bitboard of attack squares for `bitboard` of pawns with color `color`. */
static inline Bitboard pawn_attacks_bitboard(Bitboard bitboard, Color color) {
    assert(is_valid_color(color));

    return (color == COLOR_WHITE) ? white_pawn_attacks_bitboard(bitboard) : black_pawn_attacks_bitboard(bitboard);
}


/* Returns a bitboard of the base attacks of a white pawn on `square`. */
static inline Bitboard piece_base_attacks(PieceType piece_type, Square square) {
    assert(is_valid_piece_type(piece_type));
    assert(is_valid_square(square));

    return piece_base_attack_table[piece_type][square];
}


static inline unsigned magic_index(const struct Magic* magic, Bitboard occupancy) {
    assert(magic != NULL);

    return (unsigned)(((magic->mask & occupancy) * magic->factor) >> magic->shift);
}

/* Returns a bitboard of all attacks of a bishop on `square` given `occupancy`. */
static inline Bitboard bishop_attacks(Square square, Bitboard occupancy) {
    assert(is_valid_square(square));

    const struct Magic* magic = bishop_magic_table + square;

    return magic->attack_table[magic_index(magic, occupancy)];
}

/* Returns a bitboard of all attacks of a rook on `square` given `occupancy`. */
static inline Bitboard rook_attacks(Square square, Bitboard occupancy) {
    assert(is_valid_square(square));

    const struct Magic* magic = rook_magic_table + square;

    return magic->attack_table[magic_index(magic, occupancy)];
}


/* Initializes several often used bitboard lookup tables. */
extern void initialize_bitboards();


/* Returns a chessboard representation of `bitboard`. `size_out` can be ignored by passing `NULL`. */
char* bitboard_to_string(Bitboard bitboard, size_t* size_out);



#endif /* BITBOARD_H */
