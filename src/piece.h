#ifndef WINDMOLEN_PIECE_H_
#define WINDMOLEN_PIECE_H_


#include <stdint.h>

#include "util.h"



// We use an enum for colors as opposed to a boolean, because it allows us to use clever tricks to quickly determine the
// color/type of a piece and create a piece of a given color and type.
enum Color : uint8_t {
    COLOR_WHITE = 0,
    COLOR_BLACK = 1,

    COLOR_COUNT
};
static_assert(COLOR_COUNT == 2);

// Returns whether `color` is valid. This is the case when `color` is either COLOR_WHITE or COLOR_BLACK.
static INLINE bool is_valid_color(const enum Color color) {
    return color < COLOR_COUNT;
}

// Returns the opposite of `color`, assuming `color` is valid.
static INLINE enum Color opposite_color(const enum Color color) {
    assert(is_valid_color(color));

    // bitwise xor-ing with 1 switches between 0 and 1 (aka COLOR_WHTIE and COLOR_BLACK).
    return color ^ 1;
}


// Besides the enum values for all piece types, we also have a separate value for white and black pawns. This is because
// we need an extra entry for black pawns in the piece base attacks table.
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
static_assert(PIECE_TYPE_COUNT == 7);

// Returns whether `piece_type` is valid. This is the case if `piece_type` is one of
// PIECE_TYPE_PAWN,...,PIECE_TYPE_KING,PIECE_TYPE_BLACK_PAWN.
static INLINE bool is_valid_piece_type(const enum PieceType piece_type) {
    return piece_type < PIECE_TYPE_COUNT;
}


// We use these specific definitions for piece enums as they allow us use clever tricks to quickly determine the
// color/type of a piece and create a piece of a given color and type.
enum Piece : uint8_t {
    PIECE_WHITE_PAWN   = (PIECE_TYPE_PAWN << 1) + COLOR_WHITE,
    PIECE_WHITE_KNIGHT = (PIECE_TYPE_KNIGHT << 1) + COLOR_WHITE,
    PIECE_WHITE_BISHOP = (PIECE_TYPE_BISHOP << 1) + COLOR_WHITE,
    PIECE_WHITE_ROOK   = (PIECE_TYPE_ROOK << 1) + COLOR_WHITE,
    PIECE_WHITE_QUEEN  = (PIECE_TYPE_QUEEN << 1) + COLOR_WHITE,
    PIECE_WHITE_KING   = (PIECE_TYPE_KING << 1) + COLOR_WHITE,

    PIECE_BLACK_PAWN   = (PIECE_TYPE_PAWN << 1) + COLOR_BLACK,
    PIECE_BLACK_KNIGHT = (PIECE_TYPE_KNIGHT << 1) + COLOR_BLACK,
    PIECE_BLACK_BISHOP = (PIECE_TYPE_BISHOP << 1) + COLOR_BLACK,
    PIECE_BLACK_ROOK   = (PIECE_TYPE_ROOK << 1) + COLOR_BLACK,
    PIECE_BLACK_QUEEN  = (PIECE_TYPE_QUEEN << 1) + COLOR_BLACK,
    PIECE_BLACK_KING   = (PIECE_TYPE_KING << 1) + COLOR_BLACK,

    PIECE_COUNT,

    PIECE_NONE = PIECE_COUNT
};
static_assert(PIECE_COUNT == 12);

// Returns whether `piece` is valid. This is the case if `piece` is one of
// PIECE_WHITE_PAWN,...,PIECE_WHITE_KING,PIECE_BLACK_PAWN,...,PIECE_BLACK_KING.
static INLINE bool is_valid_piece(const enum Piece piece) {
    return piece < PIECE_COUNT;
}

// Returns a piece with `color` and `piece_type`, assuming `color` and `piece_type` are valid. `piece_type` should
// not be `PIECE_TYPE_BLACK_PAWN`.
static INLINE enum Piece create_piece(const enum Color color, const enum PieceType piece_type) {
    assert(is_valid_color(color));
    assert(is_valid_piece_type(piece_type));
    assert(piece_type != PIECE_TYPE_BLACK_PAWN);

    // We might be tempted to bitwise-or the color, however, since the compiler does not know that color is either 0 or
    // 1, it is unable to optimize the code. By adding the color, the compiler is able to compile to a single LEA
    // instruction: https://godbolt.org/z/x3q97av94
    return (enum Piece)((piece_type << 1) + color);
}

// Returns a piece with the same type, but opposite color as `piece`, assuming `piece` is valid.
static INLINE enum Piece opposite_piece(const enum Piece piece) {
    assert(is_valid_piece(piece));

    return piece ^ 1;  // Same trick as in opposite_color().
}

// Returns the color of `piece`, assuming `piece` is valid.
static INLINE enum Color color_of_piece(const enum Piece piece) {
    assert(is_valid_piece(piece));
    static_assert(COLOR_BLACK == 1);

    // The least significant bit of piece indicates its color. We can therefore bitwise-and with COLORBLACK == 1 to get
    // the color.
    return (enum Color)piece & COLOR_BLACK;
}

// Returns the type of `piece`, assuming `piece` is valid.
static INLINE enum PieceType type_of_piece(const enum Piece piece) {
    assert(is_valid_piece(piece));

    // Bitshifting 1 to the right gives us the type of piece because of the way we defined the piece enum values.
    return (enum PieceType)(piece >> 1);
}

// Returns the pawn type that corresponds to `color`, assuming `color` is valid.
static INLINE enum PieceType pawn_type_from_color(const enum Color color) {
    assert(is_valid_color(color));
    static_assert(PIECE_TYPE_BLACK_PAWN == 6);

    // Notice this works since PIECE_TYPE_WHITE_PAWN == 0 and COLOR_WHITE == 0. A multiplication is faster here than a
    // ternary because we multiply by 6 == PIECE_TYPE_BLACK_PAWN which compiles down to two LEA instructions:
    // https://godbolt.org/z/jPxjT85vn
    return (enum PieceType)color * PIECE_TYPE_BLACK_PAWN;
}



#endif /* #ifndef WINDMOLEN_PIECE_H_ */
