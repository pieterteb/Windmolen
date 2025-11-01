#ifndef WINDMOLEN_TYPES_H_
#define WINDMOLEN_TYPES_H_


#include <assert.h>
#include <stdbool.h>
#include <stdint.h>



typedef uint64_t Bitboard;


typedef int8_t Color;
enum {
    COLOR_WHITE = 0,
    COLOR_BLACK = 1,

    COLOR_COUNT
};
static_assert(COLOR_COUNT == 2);

/* Returns whether `color` is valid. */
static inline bool is_valid_color(Color color) {
    return color == COLOR_WHITE || color == COLOR_BLACK;
}

static inline Color opposite_color(Color color) {
    assert(is_valid_color(color));

    return color ^ 1;
}


typedef int8_t PieceType;
enum {
    PIECE_TYPE_PAWN,
    PIECE_TYPE_WHITE_PAWN = PIECE_TYPE_PAWN,
    PIECE_TYPE_KNIGHT,
    PIECE_TYPE_BISHOP,
    PIECE_TYPE_ROOK,
    PIECE_TYPE_QUEEN,
    PIECE_TYPE_KING,
    PIECE_TYPE_BLACK_PAWN,

    PIECE_TYPE_COUNT
};
static_assert(PIECE_TYPE_COUNT == 7);

/* Returns whether `piece_type` is valid. */
static inline bool is_valid_piece_type(PieceType piece_type) {
    return piece_type >= PIECE_TYPE_PAWN && piece_type < PIECE_TYPE_COUNT;
}


typedef int8_t Piece;
enum {
    PIECE_WHITE_PAWN   = (PIECE_TYPE_WHITE_PAWN << 1) | COLOR_WHITE,
    PIECE_WHITE_KNIGHT = (PIECE_TYPE_KNIGHT << 1) | COLOR_WHITE,
    PIECE_WHITE_BISHOP = (PIECE_TYPE_BISHOP << 1) | COLOR_WHITE,
    PIECE_WHITE_ROOK   = (PIECE_TYPE_ROOK << 1) | COLOR_WHITE,
    PIECE_WHITE_QUEEN  = (PIECE_TYPE_QUEEN << 1) | COLOR_WHITE,
    PIECE_WHITE_KING   = (PIECE_TYPE_KING << 1) | COLOR_WHITE,

    PIECE_BLACK_PAWN   = (PIECE_TYPE_WHITE_PAWN << 1) | COLOR_BLACK,
    PIECE_BLACK_KNIGHT = (PIECE_TYPE_KNIGHT << 1) | COLOR_BLACK,
    PIECE_BLACK_BISHOP = (PIECE_TYPE_BISHOP << 1) | COLOR_BLACK,
    PIECE_BLACK_ROOK   = (PIECE_TYPE_ROOK << 1) | COLOR_BLACK,
    PIECE_BLACK_QUEEN  = (PIECE_TYPE_QUEEN << 1) | COLOR_BLACK,
    PIECE_BLACK_KING   = (PIECE_TYPE_KING << 1) | COLOR_BLACK,

    PIECE_COUNT,

    PIECE_NONE = PIECE_COUNT
};
static_assert(PIECE_COUNT == 12);

/* Returns whether `piece` is valid. */
static inline bool is_valid_piece(Piece piece) {
    return piece >= PIECE_WHITE_PAWN && piece < PIECE_COUNT;
}

/* Returns piece of type `piece_type` and with color `color`. */
static inline Piece create_piece(Color color, PieceType piece_type) {
    assert(is_valid_color(color));
    assert(is_valid_piece_type(piece_type));
    assert(piece_type != PIECE_TYPE_BLACK_PAWN);

    return (Piece)(2 * piece_type + color);  // Fast version of (piece_type << 1) | color.
}

/* Returns the color of `piece`. */
static inline Color color_of_piece(Piece piece) {
    assert(is_valid_piece(piece));

    return (Color)(piece & COLOR_BLACK);
}

/* Returns the piece type of `piece`. */
static inline PieceType type_of_piece(Piece piece) {
    assert(is_valid_piece(piece));

    return (PieceType)(piece >> 1);
}

/* Returns pawn type of `color`. */
static inline PieceType type_of_pawn(Color color) {
    assert(is_valid_color(color));

    return PIECE_TYPE_WHITE_PAWN + color * PIECE_TYPE_BLACK_PAWN;
}


typedef int8_t CastlingRights;
enum {
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
static_assert(CASTLE_COUNT == 16);


// clang-format off
typedef int8_t Square;
enum {
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
static_assert(SQUARE_COUNT == 64);

/* Returns whether `square` is valid. */
static inline bool is_valid_square(Square square) {
    return square >= SQUARE_A1 && square <= SQUARE_H8;
}


typedef int8_t File;
enum {
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
static_assert(FILE_COUNT == 8);

/* Returns whether `file` is valid. */
static inline bool is_valid_file(File file) {
    return file >= FILE_A && file <= FILE_H;
}

/* Returns the file corresponding to the character `c`. */
static inline File char_to_file(char c) {
    assert(c >= 'a' && c <= 'h');

    return (File)(c - 'a');
}


typedef int8_t Rank;
enum {
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
static_assert(RANK_COUNT == 8);

/* Returns whether `rank` is valid. */
static inline bool is_valid_rank(Rank rank) {
    return rank >= RANK_1 && rank <= RANK_8;
}

/* Returns the rank corresponding to the character `c`. */
static inline Rank char_to_rank(char c) {
    assert(c >= '1' && c <= '8');

    return (Rank)(c - '1');
}


typedef int8_t Direction;
enum {
    DIRECTION_NORTH = 8,
    DIRECTION_EAST  = 1,
    DIRECTION_SOUTH = -DIRECTION_NORTH,
    DIRECTION_WEST  = -DIRECTION_EAST,

    DIRECTION_NORTHEAST = DIRECTION_NORTH + DIRECTION_EAST,
    DIRECTION_SOUTHEAST = DIRECTION_SOUTH + DIRECTION_EAST,
    DIRECTION_SOUTHWEST = DIRECTION_SOUTH + DIRECTION_WEST,
    DIRECTION_NORTHWEST = DIRECTION_NORTH + DIRECTION_WEST
};



#endif /* #ifndef WINDMOLEN_TYPES_H_ */
