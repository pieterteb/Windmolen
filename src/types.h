#ifndef WINDMOLEN_TYPES_H_
#define WINDMOLEN_TYPES_H_


#include <assert.h>
#include <stdint.h>

#include "util.h"



enum Color : uint8_t {
    COLOR_WHITE = 0,
    COLOR_BLACK = 1,

    COLOR_COUNT
};
static_assert(COLOR_COUNT == 2, "There should be exactly two colors: white and black.");

// Returns whether `color` is valid.
static INLINE bool is_valid_color(enum Color color) {
    return color < COLOR_COUNT;
}

// Returns the opposite of `color`.
static INLINE enum Color opposite_color(enum Color color) {
    assert(is_valid_color(color));

    return color ^ 1;  // bitwise xor-ing with 1 switches between 0 and 1 (aka COLOR_WHTIE and COLOR_BLACK).
}


enum PieceType : uint8_t {
    PIECE_TYPE_PAWN       = 0,
    PIECE_TYPE_WHITE_PAWN = PIECE_TYPE_PAWN,
    PIECE_TYPE_KNIGHT     = 1,
    PIECE_TYPE_BISHOP     = 2,
    PIECE_TYPE_ROOK       = 3,
    PIECE_TYPE_QUEEN      = 4,
    PIECE_TYPE_KING       = 5,
    PIECE_TYPE_BLACK_PAWN = 6,

    PIECE_TYPE_COUNT
};
static_assert(PIECE_TYPE_COUNT == 7,
              "There should be exactly 7 different piece types: 6 for each type of piece, and one extra value to "
              "differentiate between white and black pawns.");

// Returns whether `piece_type` is valid.
static INLINE bool is_valid_piece_type(enum PieceType piece_type) {
    return piece_type < PIECE_TYPE_COUNT;
}


enum Piece : uint8_t {
    PIECE_WHITE_PAWN   = (PIECE_TYPE_WHITE_PAWN << 1) + COLOR_WHITE,
    PIECE_WHITE_KNIGHT = (PIECE_TYPE_KNIGHT << 1) + COLOR_WHITE,
    PIECE_WHITE_BISHOP = (PIECE_TYPE_BISHOP << 1) + COLOR_WHITE,
    PIECE_WHITE_ROOK   = (PIECE_TYPE_ROOK << 1) + COLOR_WHITE,
    PIECE_WHITE_QUEEN  = (PIECE_TYPE_QUEEN << 1) + COLOR_WHITE,
    PIECE_WHITE_KING   = (PIECE_TYPE_KING << 1) + COLOR_WHITE,

    PIECE_BLACK_PAWN   = (PIECE_TYPE_WHITE_PAWN << 1) + COLOR_BLACK,
    PIECE_BLACK_KNIGHT = (PIECE_TYPE_KNIGHT << 1) + COLOR_BLACK,
    PIECE_BLACK_BISHOP = (PIECE_TYPE_BISHOP << 1) + COLOR_BLACK,
    PIECE_BLACK_ROOK   = (PIECE_TYPE_ROOK << 1) + COLOR_BLACK,
    PIECE_BLACK_QUEEN  = (PIECE_TYPE_QUEEN << 1) + COLOR_BLACK,
    PIECE_BLACK_KING   = (PIECE_TYPE_KING << 1) + COLOR_BLACK,

    PIECE_COUNT,

    PIECE_NONE = PIECE_COUNT
};
static_assert(PIECE_COUNT == 12, "There should be exactly 12 different pieces: 6 types for each color.");

// Returns whether `piece` is valid.
static INLINE bool is_valid_piece(enum Piece piece) {
    return piece < PIECE_COUNT;
}

// Returns a piece with `color` and `piece_type`. The type of piece should not be `PIECE_TYPE_BLACK_PAWN`.
static INLINE enum Piece create_piece(enum Color color, enum PieceType piece_type) {
    assert(is_valid_color(color));
    assert(is_valid_piece_type(piece_type));
    assert(piece_type != PIECE_TYPE_BLACK_PAWN);

    return (enum Piece)((piece_type << 1) + color);
}

// Returns the color of `piece`.
static INLINE enum Color color_of_piece(enum Piece piece) {
    assert(is_valid_piece(piece));

    return (enum Color)piece & COLOR_BLACK;
}

// Returns the type of `piece`.
static INLINE enum PieceType type_of_piece(enum Piece piece) {
    assert(is_valid_piece(piece));

    return (enum PieceType)(piece >> 1);
}

// Returns the pawn type that corresponds to `color`.
static INLINE enum PieceType type_of_pawn(enum Color color) {
    assert(is_valid_color(color));

    // Notice this works since PIECE_TYPE_WHITE_PAWN == 0 and COLOR_WHITE == 0. A
    // multiplication is faster here than a ternary because we
    // multiply by 6 which compiles down to two LEA instructions.
    // https://godbolt.org/z/jPxjT85vn
    return (enum PieceType)color * PIECE_TYPE_BLACK_PAWN;
}


enum CastlingRights : uint8_t {
    CASTLE_NONE       = 0,
    CASTLE_WHITE_00   = 1,
    CASTLE_WHITE_000  = CASTLE_WHITE_00 << 1,
    CASTLE_BLACK_00   = CASTLE_WHITE_00 << 2,
    CASTLE_BLACK_000  = CASTLE_WHITE_00 << 3,
    CASTLE_KING_SIDE  = CASTLE_WHITE_00 | CASTLE_BLACK_00,
    CASTLE_QUEEN_SIDE = CASTLE_WHITE_000 | CASTLE_BLACK_000,
    CASTLE_WHITE      = CASTLE_WHITE_00 | CASTLE_WHITE_000,
    CASTLE_BLACK      = CASTLE_BLACK_00 | CASTLE_BLACK_000,
    CASTLE_ANY        = CASTLE_WHITE | CASTLE_BLACK,

    CASTLE_COUNT
};
static_assert(CASTLE_COUNT == 16, "There should be 16 different castling rights configurations.");


// clang-format off
enum Square : uint8_t {
    SQUARE_A1, SQUARE_B1, SQUARE_C1, SQUARE_D1, SQUARE_E1, SQUARE_F1, SQUARE_G1, SQUARE_H1,
    SQUARE_A2, SQUARE_B2, SQUARE_C2, SQUARE_D2, SQUARE_E2, SQUARE_F2, SQUARE_G2, SQUARE_H2,
    SQUARE_A3, SQUARE_B3, SQUARE_C3, SQUARE_D3, SQUARE_E3, SQUARE_F3, SQUARE_G3, SQUARE_H3,
    SQUARE_A4, SQUARE_B4, SQUARE_C4, SQUARE_D4, SQUARE_E4, SQUARE_F4, SQUARE_G4, SQUARE_H4,
    SQUARE_A5, SQUARE_B5, SQUARE_C5, SQUARE_D5, SQUARE_E5, SQUARE_F5, SQUARE_G5, SQUARE_H5,
    SQUARE_A6, SQUARE_B6, SQUARE_C6, SQUARE_D6, SQUARE_E6, SQUARE_F6, SQUARE_G6, SQUARE_H6,
    SQUARE_A7, SQUARE_B7, SQUARE_C7, SQUARE_D7, SQUARE_E7, SQUARE_F7, SQUARE_G7, SQUARE_H7,
    SQUARE_A8, SQUARE_B8, SQUARE_C8, SQUARE_D8, SQUARE_E8, SQUARE_F8, SQUARE_G8, SQUARE_H8,

    SQUARE_COUNT,

    SQUARE_NONE = SQUARE_COUNT
};
// clang-format on
static_assert(SQUARE_COUNT == 64, "There should be exactly 64 squares on a chessboard.");

// Returns whether `square` is valid.
static INLINE bool is_valid_square(enum Square square) {
    return square < SQUARE_COUNT;
}


enum File : uint8_t {
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H,

    FILE_COUNT
};
static_assert(FILE_COUNT == 8, "There should be exactly 8 different files: A through H.");

// Returns whether `file` is valid.
static INLINE bool is_valid_file(enum File file) {
    return file < FILE_COUNT;
}

// Returns the file that `square` lies on.
static INLINE enum File file_of_square(enum Square square) {
    assert(is_valid_square(square));

    return square & 7;  // Fast modulo 8.
}

// Returns the file corresponding to `c` (lowercase).
static INLINE enum File char_to_file(char c) {
    assert(c >= 'a' && c <= 'h');

    return (enum File)(c - 'a');
}


enum Rank : uint8_t {
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,

    RANK_COUNT
};
static_assert(RANK_COUNT == 8, "There should be exactly 8 different files: 1 through 8.");

// Returns whether `rank` is valid.
static INLINE bool is_valid_rank(enum Rank rank) {
    return rank < RANK_COUNT;
}

// Returns the rank that `square` lies on.
static INLINE enum Rank rank_of_square(enum Square square) {
    assert(is_valid_square(square));

    return square >> 3;  // Fast division by 8.
}

// Returns the rank corresponding to `c` (lowercase).
static INLINE enum Rank char_to_rank(char c) {
    assert(c >= '1' && c <= '8');

    return (enum Rank)(c - '1');
}


enum Direction : int8_t {
    DIRECTION_NORTH = 8,
    DIRECTION_EAST  = 1,
    DIRECTION_SOUTH = -DIRECTION_NORTH,
    DIRECTION_WEST  = -DIRECTION_EAST,

    DIRECTION_NORTHEAST = DIRECTION_NORTH + DIRECTION_EAST,
    DIRECTION_SOUTHEAST = DIRECTION_SOUTH + DIRECTION_EAST,
    DIRECTION_SOUTHWEST = DIRECTION_SOUTH + DIRECTION_WEST,
    DIRECTION_NORTHWEST = DIRECTION_NORTH + DIRECTION_WEST,

    DIRECTION_NORTH2 = 2 * DIRECTION_NORTH,
    DIRECTION_SOUTH2 = 2 * DIRECTION_SOUTH
};



#endif /* #ifndef WINDMOLEN_TYPES_H_ */
