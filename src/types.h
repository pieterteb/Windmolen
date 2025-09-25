#ifndef TYPES_H
#define TYPES_H


#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "bitboard.h"



typedef uint64_t Bitboard;


typedef int8_t Color;
enum Color {
    COLOR_WHITE,
    COLOR_BLACK,

    COLOR_COUNT
};
static_assert(COLOR_COUNT == 2);

// Returns whether color is valid.
static inline bool is_valid_color(Color color) {
    return color == COLOR_WHITE || color == COLOR_BLACK;
}


typedef int8_t PieceType;
enum PieceType {
    PIECE_TYPE_PAWN,
    PIECE_TYPE_WHITE_PAWN = PIECE_TYPE_PAWN,
    PIECE_TYPE_KNIGHT,
    PIECE_TYPE_BISHOP,
    PIECE_TYPE_ROOK,
    PIECE_TYPE_QUEEN,
    PIECE_TYPE_KING,
    PIECE_TYPE_BLACK_PAWN,

    PIECE_TYPE_COUNT = 7
};
static_assert(PIECE_TYPE_COUNT == 7);

// Returns whether piece_type is valid.
static inline bool is_valid_piece_type(PieceType piece_type) {
    return piece_type >= 0 && piece_type < PIECE_TYPE_COUNT;
}


typedef int8_t Piece;
enum Piece {
    PIECE_WHITE_PAWN,
    PIECE_WHITE_KNIGHT,
    PIECE_WHITE_BISHOP,
    PIECE_WHITE_ROOK,
    PIECE_WHITE_QUEEN,
    PIECE_WHITE_KING,

    PIECE_BLACK_PAWN,
    PIECE_BLACK_KNIGHT,
    PIECE_BLACK_BISHOP,
    PIECE_BLACK_ROOK,
    PIECE_BLACK_QUEEN,
    PIECE_BLACK_KING,

    PIECE_COUNT,

    PIECE_NONE = PIECE_COUNT
};
static_assert(PIECE_BLACK_PAWN == PIECE_TYPE_BLACK_PAWN);
static_assert(PIECE_COUNT == 12);

// Returns whether piece is valid.
static inline bool is_valid_piece(Piece piece) {
    return piece >= 0 && piece < PIECE_COUNT;
}

// Piece color of piece.
static inline Color piece_color(Piece piece) {
    assert(is_valid_piece(piece));

    return (Color)(piece > PIECE_WHITE_KING);
}


typedef int8_t CastlingRights;
enum CastlingRights {
    NO_CASTLING,
    WHITE_00 = 1,
    WHITE_000 = WHITE_00 << 1,
    BLACK_00 = WHITE_00 << 2,
    BLACK_000 = WHITE_00 << 3,
    KING_SIDE = WHITE_00 | BLACK_00,
    QUEEN_SIDE = WHITE_000 | BLACK_000,
    WHITE_CASTLING = WHITE_00 | WHITE_000,
    BLACK_CASTLING = BLACK_00 | BLACK_000,
    ANY_CASTLING = WHITE_CASTLING | BLACK_CASTLING
};


typedef int8_t Square;
enum Square {
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
static_assert(SQUARE_COUNT == 64);

// Returns whether square is valid.
static inline bool is_valid_square(Square square) {
    return square >= SQUARE_A1 && square <= SQUARE_H8;
}


typedef int8_t File;
enum File {
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

// Returns whether file is valid.
static inline bool is_valid_file(File file) {
    return file >= FILE_A && file <= FILE_H;
}

static inline File char_to_file(char c) {
    assert(c >= 'a' && c <= 'h');

    return (File)(c - 'a');
}


typedef int8_t Rank;
enum Rank {
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

// Returns whether rank is valid.
static inline bool is_valid_rank(Rank rank) {
    return rank >= RANK_1 && rank <= RANK_8;
}

static inline Rank char_to_rank(char c) {
    assert(c >= '1' && c <= '8');

    return (Rank)(c - '1');
}


typedef int8_t Direction;
enum Direction {
    DIRECTION_NORTH = 8,
    DIRECTION_EAST = 1,
    DIRECTION_SOUTH = -DIRECTION_NORTH,
    DIRECTION_WEST = -DIRECTION_EAST,

    DIRECTION_NORTHEAST = DIRECTION_NORTH + DIRECTION_EAST,
    DIRECTION_SOUTHEAST = DIRECTION_SOUTH + DIRECTION_EAST,
    DIRECTION_SOUTHWEST = DIRECTION_SOUTH + DIRECTION_WEST,
    DIRECTION_NORTHWEST = DIRECTION_NORTH + DIRECTION_WEST
};



#endif /* TYPES_H */
