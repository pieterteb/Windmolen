#ifndef WINDMOLEN_BITBOARD_H_
#define WINDMOLEN_BITBOARD_H_


#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "board.h"
#include "util.h"



typedef uint64_t Bitboard;
constexpr Bitboard EMPTY_BITBOARD = 0;

// File masks.
static constexpr Bitboard FILE_A_BITBOARD = 0x0101010101010101;
static constexpr Bitboard FILE_B_BITBOARD = FILE_A_BITBOARD << FILE_B;
static constexpr Bitboard FILE_C_BITBOARD = FILE_A_BITBOARD << FILE_C;
static constexpr Bitboard FILE_D_BITBOARD = FILE_A_BITBOARD << FILE_D;
static constexpr Bitboard FILE_E_BITBOARD = FILE_A_BITBOARD << FILE_E;
static constexpr Bitboard FILE_F_BITBOARD = FILE_A_BITBOARD << FILE_F;
static constexpr Bitboard FILE_G_BITBOARD = FILE_A_BITBOARD << FILE_G;
static constexpr Bitboard FILE_H_BITBOARD = FILE_A_BITBOARD << FILE_H;

// Rank masks.
static constexpr Bitboard RANK_1_BITBOARD = 0x00000000000000ff;
static constexpr Bitboard RANK_2_BITBOARD = RANK_1_BITBOARD << (RANK_2 * 8);
static constexpr Bitboard RANK_3_BITBOARD = RANK_1_BITBOARD << (RANK_3 * 8);
static constexpr Bitboard RANK_4_BITBOARD = RANK_1_BITBOARD << (RANK_4 * 8);
static constexpr Bitboard RANK_5_BITBOARD = RANK_1_BITBOARD << (RANK_5 * 8);
static constexpr Bitboard RANK_6_BITBOARD = RANK_1_BITBOARD << (RANK_6 * 8);
static constexpr Bitboard RANK_7_BITBOARD = RANK_1_BITBOARD << (RANK_7 * 8);
static constexpr Bitboard RANK_8_BITBOARD = RANK_1_BITBOARD << (RANK_8 * 8);


extern const Bitboard diagonal_bitboards[SQUARE_COUNT];
extern const Bitboard antidiagonal_bitboards[SQUARE_COUNT];
extern const Bitboard line_bitboards[SQUARE_COUNT][SQUARE_COUNT];
extern const Bitboard between_bitboards[SQUARE_COUNT][SQUARE_COUNT];
extern const Bitboard piece_base_attacks_table[PIECE_TYPE_COUNT][SQUARE_COUNT];


// Returns a bitboard of the entire diagonal on which `square` lies, assuming `square` is valid.
static INLINE Bitboard diagonal_bitboard(const enum Square square) {
    assert(is_valid_square(square));

    return diagonal_bitboards[square];
}

// Returns a bitboard of the entire antidiagonal on which `square` lies, assuming `square` is valid.
static INLINE Bitboard antidiagonal_bitboard(const enum Square square) {
    assert(is_valid_square(square));

    return antidiagonal_bitboards[square];
}

// Returns a bitboard of an entire line that intersects `square1` and `square2`. If the squares do not lie on the same
// file/rank/diagonal/antidiagonal, the function returns `EMPTY_BITBOARD`. We assume `square1` and `square2` are both
// valid squares.
static INLINE Bitboard line_bitboard(const enum Square square1, const enum Square square2) {
    assert(is_valid_square(square1));
    assert(is_valid_square(square2));

    return line_bitboards[square1][square2];  // Precomputed by Windmolen/tools/lookup_tables/line_bitboards.py.
}

// Returns a bitboard of the squares in the semi-open segment between `square1` and `square2` (excluding `square1`,
// including `square2`). If the squares do not lie on the same file/rank/diagonal/antidiagonal, the function returns a
// bitboard of `square2`. This way, we are able to compute check evasion moves without the king faster, as the piece
// that is moved must either interpose or capture the attacker. We assume `square1` and `square2` are valid squares.
static INLINE Bitboard between_bitboard(const enum Square square1, const enum Square square2) {
    assert(is_valid_square(square1));
    assert(is_valid_square(square2));

    return between_bitboards[square1][square2];  // Precomputed by Windmolen/tools/lookup_tables/between_bitboards.py.
}

// Returns a bitboard of the base attacks of a piece of type `piece_type` on `square`, assuming `piece_type` and
// `square` are valid.
static INLINE Bitboard piece_base_attacks(const enum PieceType piece_type, const enum Square square) {
    assert(is_valid_piece_type(piece_type));
    assert(is_valid_square(square));

    return piece_base_attacks_table[piece_type][square];  // Precomputed by
                                                          // Windmolen/tools/lookup_tables/piece_base_attacks_table.py.
}


// Returns a bitboard of `square`, assuming `square` is valid.
static INLINE Bitboard square_bitboard(const enum Square square) {
    assert(is_valid_square(square));

    return (Bitboard)1 << square;
}

// Static evaluation version of `square_bitboard()`.
#define SQUARE_BITBOARD(square) ((Bitboard)1 << (square))

// Returns a bitboard of `file`, assuming `file` is valid.
static INLINE Bitboard file_bitboard(const enum File file) {
    assert(is_valid_file(file));

    return FILE_A_BITBOARD << file;
}

// Returns a bitboard of `rank`, assuming `rank` is valid.
static INLINE Bitboard rank_bitboard(const enum Rank rank) {
    assert(is_valid_rank(rank));

    return RANK_1_BITBOARD << (rank << 3);  // << 3: fast multiplication by 8.
}

// Returns a bitboard of the file that `square` lies on, assuming `square` is valid.
static INLINE Bitboard file_bitboard_from_square(const enum Square square) {
    assert(is_valid_square(square));

    return file_bitboard(file_of_square(square));
}

// Returns a bitboard of the rank that `square` lies on, assuming `square` is valid.
static INLINE Bitboard rank_bitboard_from_square(const enum Square square) {
    assert(is_valid_square(square));

    return rank_bitboard(rank_of_square(square));
}

// Returns a bitboard of the square described by `file` and `rank`, assuming `file` and `rank` are valid.
static INLINE Bitboard bitboard_from_coordinates(const enum File file, const enum Rank rank) {
    assert(is_valid_file(file));
    assert(is_valid_rank(rank));

    return file_bitboard(file) & rank_bitboard(rank);
}


// Shifts `bitboard` north.
static INLINE Bitboard shift_bitboard_north(const Bitboard bitboard) {
    return bitboard << 8;
}

// Shifts `bitboard` north twice.
static INLINE Bitboard shift_bitboard_2north(const Bitboard bitboard) {
    return bitboard << 16;
}

// Shifts `bitboard` south.
static INLINE Bitboard shift_bitboard_south(const Bitboard bitboard) {
    return bitboard >> 8;
}

// Shifts `bitboard` south twice.
static INLINE Bitboard shift_bitboard_2south(const Bitboard bitboard) {
    return bitboard >> 16;
}

// Shifts `bitboard` east.
static INLINE Bitboard shift_bitboard_east(const Bitboard bitboard) {
    return (bitboard & ~FILE_H_BITBOARD) << 1;
}

// Shifts `bitboard` west.
static INLINE Bitboard shift_bitboard_west(const Bitboard bitboard) {
    return (bitboard & ~FILE_A_BITBOARD) >> 1;
}

// Shifts `bitboard` northeast.
static INLINE Bitboard shift_bitboard_northeast(const Bitboard bitboard) {
    return (bitboard & ~FILE_H_BITBOARD) << 9;
}

// Shifts `bitboard` southeast.
static INLINE Bitboard shift_bitboard_southeast(const Bitboard bitboard) {
    return (bitboard & ~FILE_H_BITBOARD) >> 7;
}

// Shifts `bitboard` southwest.
static INLINE Bitboard shift_bitboard_southwest(const Bitboard bitboard) {
    return (bitboard & ~FILE_A_BITBOARD) >> 9;
}

// Shifts `bitboard` northwest.
static INLINE Bitboard shift_bitboard_northwest(const Bitboard bitboard) {
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
static INLINE size_t magic_index(const struct Magic* magic, const Bitboard occupancy) {
    assert(magic != nullptr);

    return (unsigned)(((magic->mask & occupancy) * magic->factor) >> magic->shift);
}

// Returns a bitboard of all attacks of a bishop on `square` given `occupancy`, assuming `square` is valid.
static inline Bitboard bishop_attacks(const enum Square square, const Bitboard occupancy) {
    assert(is_valid_square(square));

    const struct Magic* magic = bishop_magic_table + square;

    return magic->attack_table[magic_index(magic, occupancy)];
}

// Returns a bitboard of all attacks of a rook on `square` given `occupancy`, assuming `square` is valid.
static inline Bitboard rook_attacks(const enum Square square, const Bitboard occupancy) {
    assert(is_valid_square(square));

    const struct Magic* magic = rook_magic_table + square;

    return magic->attack_table[magic_index(magic, occupancy)];
}


// Returns a bitboard of the attacks of a piece of type `piece_type` on `square` with `occupancy`, assuming `piece_type`
// and `square` are valid.
static INLINE Bitboard piece_attacks(const enum PieceType piece_type, const enum Square square,
                                     const Bitboard occupancy) {
    assert(is_valid_piece_type(piece_type));
    assert(is_valid_square(square));

    switch (piece_type) {
        case PIECE_TYPE_BISHOP:
            return bishop_attacks(square, occupancy);
        case PIECE_TYPE_ROOK:
            return rook_attacks(square, occupancy);
        case PIECE_TYPE_QUEEN:
            return bishop_attacks(square, occupancy) | rook_attacks(square, occupancy);
        default:
            return piece_base_attacks(piece_type, square);
    }
}


// Initializes several bitboard lookup tables.
extern void initialize_bitboards();


// Prints `bitboard` in a human readable format to `stdout`. Useful for debugging.
[[maybe_unused]] void print_bitboard(const Bitboard bitboard);



#endif /* #ifndef WINDMOLEN_BITBOARD_H_ */
