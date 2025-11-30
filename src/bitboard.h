#ifndef WINDMOLEN_BITBOARD_H_
#define WINDMOLEN_BITBOARD_H_


#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "types.h"
#include "util.h"



typedef uint64_t Bitboard;
constexpr Bitboard EMPTY_BITBOARD = (Bitboard)0;

// File masks.
static constexpr Bitboard FILE_A_BITBOARD = 0x0101010101010101;
static constexpr Bitboard FILE_B_BITBOARD = FILE_A_BITBOARD << 1;
static constexpr Bitboard FILE_C_BITBOARD = FILE_A_BITBOARD << 2;
static constexpr Bitboard FILE_D_BITBOARD = FILE_A_BITBOARD << 3;
static constexpr Bitboard FILE_E_BITBOARD = FILE_A_BITBOARD << 4;
static constexpr Bitboard FILE_F_BITBOARD = FILE_A_BITBOARD << 5;
static constexpr Bitboard FILE_G_BITBOARD = FILE_A_BITBOARD << 6;
static constexpr Bitboard FILE_H_BITBOARD = FILE_A_BITBOARD << 7;

// Rank masks.
static constexpr Bitboard RANK_1_BITBOARD = 0x00000000000000ff;
static constexpr Bitboard RANK_2_BITBOARD = RANK_1_BITBOARD << (1 * 8);
static constexpr Bitboard RANK_3_BITBOARD = RANK_1_BITBOARD << (2 * 8);
static constexpr Bitboard RANK_4_BITBOARD = RANK_1_BITBOARD << (3 * 8);
static constexpr Bitboard RANK_5_BITBOARD = RANK_1_BITBOARD << (4 * 8);
static constexpr Bitboard RANK_6_BITBOARD = RANK_1_BITBOARD << (5 * 8);
static constexpr Bitboard RANK_7_BITBOARD = RANK_1_BITBOARD << (6 * 8);
static constexpr Bitboard RANK_8_BITBOARD = RANK_1_BITBOARD << (7 * 8);

extern const uint8_t square_distances[SQUARE_COUNT][SQUARE_COUNT];

extern const Bitboard line_bitboards[SQUARE_COUNT][SQUARE_COUNT];
extern const Bitboard between_bitboards[SQUARE_COUNT][SQUARE_COUNT];
extern const Bitboard piece_base_attacks_table[PIECE_TYPE_COUNT][SQUARE_COUNT];


/* The distance between x and y is defined as the number of king moves required to go from x to y. */

// Returns the distance between `square1` and `square2`.
static INLINE uint8_t distance(enum Square square1, enum Square square2) {
    assert(is_valid_square(square1));
    assert(is_valid_square(square2));

    return square_distances[square1][square2];  // Precomputed by ../tools/lookup_table/square_distances.py.
}

// Returns the distance between the files that `square1` and `square2` lie on.
static INLINE uint8_t file_distance(enum Square square1, enum Square square2) {
    assert(is_valid_square(square1));
    assert(is_valid_square(square2));

    return (uint8_t)abs(file_of_square(square1) - file_of_square(square2));
}

// Returns the distance between the ranks that `square1` and `square2` lie on.
static INLINE uint8_t rank_distance(enum Square square1, enum Square square2) {
    assert(is_valid_square(square1));
    assert(is_valid_square(square2));

    return (uint8_t)abs(rank_of_square(square1) - rank_of_square(square2));
}


// Returns a bitboard of an entire line that intersects `square1` and `square2`. If the squares do not lie on the same
// file/rank/diagonal/antidiagonal, the function returns `EMPTY_BITBOARD`.
static INLINE Bitboard line_bitboard(enum Square square1, enum Square square2) {
    assert(is_valid_square(square1));
    assert(is_valid_square(square2));

    return line_bitboards[square1][square2];  // Precomputed by Windmolen/tools/lookup_tables/line_bitboards.py.
}

// Returns a bitboard of the squares in the semi-open segment between `square1` and `square2` (excluding `square1`,
// including `square2`). If the squares do not lie on the same file/rank/diagonal/antidiagonal, the function returns a
// bitboard of `square2`. This way, we are able to compute check evasion moves without the king faster, as the piece
// that is moved must either interpose or capture the attacker.
static INLINE Bitboard between_bitboard(enum Square square1, enum Square square2) {
    assert(is_valid_square(square1));
    assert(is_valid_square(square2));

    return between_bitboards[square1][square2];  // Precomputed by Windmolen/tools/lookup_tables/between_bitboards.py.
}

// Returns a bitboard of the base attacks of a piece of type `piece_type` on `square`.
static INLINE Bitboard piece_base_attacks(enum PieceType piece_type, enum Square square) {
    assert(is_valid_piece_type(piece_type));
    assert(is_valid_square(square));

    return piece_base_attacks_table[piece_type]
                                   [square];  // Precomputed by Windmolen/tools/lookup_tables/piece_base_attacks_table.py.
}


// Returns a bitboard of `square`.
static INLINE Bitboard square_bitboard(enum Square square) {
    assert(is_valid_square(square));

    return (Bitboard)1 << square;
}

// Static evaluation version of `square_bitboard()`.
#define SQUARE_BITBOARD(square) ((Bitboard)1 << (square))

// Returns a bitboard of `file`.
static INLINE Bitboard file_bitboard(enum File file) {
    assert(is_valid_file(file));

    return FILE_A_BITBOARD << file;
}

// Returns a bitboard of `rank`.
static INLINE Bitboard rank_bitboard(enum Rank rank) {
    assert(is_valid_rank(rank));

    return RANK_1_BITBOARD << (rank << 3);  // << 3: fast multiplication by 8.
}

// Returns a bitboard of the file that `square` lies on.
static INLINE Bitboard file_bitboard_from_square(enum Square square) {
    assert(is_valid_square(square));

    return file_bitboard(file_of_square(square));
}

// Returns a bitboard of the rank that `square` lies on.
static INLINE Bitboard rank_bitboard_from_square(enum Square square) {
    assert(is_valid_square(square));

    return rank_bitboard(rank_of_square(square));
}

// Returns a bitboard of the square described by `file` and `rank`.
static INLINE Bitboard bitboard_from_coordinates(enum File file, enum Rank rank) {
    assert(is_valid_file(file));
    assert(is_valid_rank(rank));

    return file_bitboard(file) & rank_bitboard(rank);
}

// Returns the square described by `file` and `rank`.
static INLINE enum Square square_from_coordinates(enum File file, enum Rank rank) {
    assert(is_valid_file(file));
    assert(is_valid_rank(rank));

    // Should compile to a single LEA instruction on modern CPUs.
    return (enum Square)((enum Direction)file * DIRECTION_EAST + (enum Direction)rank * DIRECTION_NORTH);
}


// Shifts `bitboard` north.
static INLINE Bitboard shift_bitboard_north(Bitboard bitboard) {
    return bitboard << 8;
}

// Shifts `bitboard` north twice.
static INLINE Bitboard shift_bitboard_2north(Bitboard bitboard) {
    return bitboard << 16;
}

// Shifts `bitboard` south.
static INLINE Bitboard shift_bitboard_south(Bitboard bitboard) {
    return bitboard >> 8;
}

// Shifts `bitboard` south twice.
static INLINE Bitboard shift_bitboard_2south(Bitboard bitboard) {
    return bitboard >> 16;
}

// Shifts `bitboard` east.
static INLINE Bitboard shift_bitboard_east(Bitboard bitboard) {
    return (bitboard & ~FILE_H_BITBOARD) << 1;
}

// Shifts `bitboard` west.
static INLINE Bitboard shift_bitboard_west(Bitboard bitboard) {
    return (bitboard & ~FILE_A_BITBOARD) >> 1;
}

// Shifts `bitboard` northeast.
static INLINE Bitboard shift_bitboard_northeast(Bitboard bitboard) {
    return (bitboard & ~FILE_H_BITBOARD) << 9;
}

// Shifts `bitboard` southeast.
static INLINE Bitboard shift_bitboard_southeast(Bitboard bitboard) {
    return (bitboard & ~FILE_H_BITBOARD) >> 7;
}

// Shifts `bitboard` southwest.
static INLINE Bitboard shift_bitboard_southwest(Bitboard bitboard) {
    return (bitboard & ~FILE_A_BITBOARD) >> 9;
}

// Shifts `bitboard` northwest.
static INLINE Bitboard shift_bitboard_northwest(Bitboard bitboard) {
    return (bitboard & ~FILE_A_BITBOARD) << 7;
}


struct Magic {
    Bitboard* attack_table;
    Bitboard mask;
    Bitboard factor;
    unsigned shift;
};

extern struct Magic bishop_magic_table[SQUARE_COUNT];
extern struct Magic rook_magic_table[SQUARE_COUNT];

// Returns an index used to quickly determine bishop and rook move bitboards. This function should not be called outside
// of bitboard.h.
static INLINE size_t magic_index(const struct Magic* magic, Bitboard occupancy) {
    assert(magic != nullptr);

    return (unsigned)(((magic->mask & occupancy) * magic->factor) >> magic->shift);
}

// Returns a bitboard of all attacks of a bishop on `square` given `occupancy`.
static inline Bitboard bishop_attacks(enum Square square, Bitboard occupancy) {
    assert(is_valid_square(square));

    const struct Magic* magic = bishop_magic_table + square;

    return magic->attack_table[magic_index(magic, occupancy)];
}

// Returns a bitboard of all attacks of a rook on `square` given `occupancy`.
static inline Bitboard rook_attacks(enum Square square, Bitboard occupancy) {
    assert(is_valid_square(square));

    const struct Magic* magic = rook_magic_table + square;

    return magic->attack_table[magic_index(magic, occupancy)];
}


// Initializes several bitboard lookup tables.
extern void initialize_bitboards();


// Prints `bitboard` in a human readable format to `stdout`. Useful for debugging.
[[maybe_unused]] void print_bitboard(Bitboard bitboard);



#endif /* #ifndef WINDMOLEN_BITBOARD_H_ */
