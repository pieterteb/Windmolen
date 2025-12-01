#ifndef WINDMOLEN_MOVE_H_
#define WINDMOLEN_MOVE_H_


#include <stdint.h>

#include "board.h"
#include "piece.h"
#include "util.h"



// We use 16 bits to describe a move. We need 6 bits for the source square as well as the destination square.
// The remaining 4 bits are used for special moves:
//
//      Bits 0-5: source square (0-63),
//      Bits 6-11: destination square (0-63),
//      Bits 12-13: special bits (normal: 0, promotion: 1, castle: 2, en passant: 3),
//      Bits 14-15: promotion piece - 1
//
typedef uint16_t Move;
constexpr Move NULL_MOVE = (Move)0;


enum MoveType : Move {
    MOVE_TYPE_NORMAL     = 0 << 12,
    MOVE_TYPE_PROMOTION  = 1 << 12,
    MOVE_TYPE_CASTLE     = 2 << 12,
    MOVE_TYPE_EN_PASSANT = 3 << 12,

    KNIGHT_PROMOTION = 0 << 14,
    BISHOP_PROMOTION = 1 << 14,
    ROOK_PROMOTION   = 2 << 14,
    QUEEN_PROMOTION  = 3 << 14,

    MOVE_TYPE_KNIGHT_PROMOTION = MOVE_TYPE_PROMOTION | KNIGHT_PROMOTION,
    MOVE_TYPE_BISHOP_PROMOTION = MOVE_TYPE_PROMOTION | BISHOP_PROMOTION,
    MOVE_TYPE_ROOK_PROMOTION   = MOVE_TYPE_PROMOTION | ROOK_PROMOTION,
    MOVE_TYPE_QUEEN_PROMOTION  = MOVE_TYPE_PROMOTION | QUEEN_PROMOTION,
};

// Returns the source square of `move`.
static INLINE enum Square move_source(const Move move) {
    return move & 0X003F;  // Get first 6 bits.
}

// Returns the destination square of `move`.
static INLINE enum Square move_destination(const Move move) {
    return (move >> 6) & 0X003F;  // Get second 6 bits.
}

// Returns whether the source square of `move` is equal to its destination square.
static INLINE bool is_weird_move(const Move move) {
    return move_source(move) == move_destination(move);
}

// Returns the type of `move`.
static INLINE enum MoveType move_type(const Move move) {
    assert(!is_weird_move(move));

    return move & (3 << 12);  // Mask bits 12 and 13.
}

// Returns the character corresponding to the promotion type of `move`, assuming `move` is not weird. Calling this
// function only makes sense if the move is an actual promotion.
static INLINE char promotion_to_char(const Move move) {
    assert(!is_weird_move(move));
    assert(move_type(move) == MOVE_TYPE_PROMOTION);

    return (const char[]){'n', 'b', 'r', 'q'}[move >> 14];  // Get promotion bits (14 and 15).
}

// Returns the piece type that corresponds to the promotion type of `move`, assuming `move` is not weird and `move` is
// an actual promotion.
static INLINE enum PieceType promotion_piece_type(const Move move) {
    assert(!is_weird_move(move));
    assert(move_type(move) == MOVE_TYPE_PROMOTION);

    return (enum PieceType)((move >> 14) + 1);
}


// Returns a move from `source` to `destination` of `type`, assuming `source` and `destination` are different valid
// squares.
static INLINE Move new_move(const enum Square source, const enum Square destination, const enum MoveType type) {
    assert(is_valid_square(source));
    assert(is_valid_square(destination));
    assert(source != destination);

    // Following the convention described at the top of this file.
    return (Move)(source | (destination << 6) | type);
}

// Returns a normal move from `source` to `destination`, assuming `source` and `destination are different valid squares.
static INLINE Move new_normal_move(const enum Square source, const enum Square destination) {
    assert(is_valid_square(source));
    assert(is_valid_square(destination));
    assert(source != destination);

    return (Move)(source | (destination << 6) | MOVE_TYPE_NORMAL);
}


// The castling right enum values are masks for the 4 different castle moves: kingside and queenside for both colors.
// So, to access/update castling rights, we can use simple bitwise operations.
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
static_assert(CASTLE_COUNT == 16);

// Returns a castle move for `color` to `castle_side`, assuming `color` is valid and `castle_side` is either
// CASTLE_KING_SIDE or CASTLE_QUEEN_SIDE.
static INLINE Move new_castle(const enum Color color, const enum CastlingRights castle_side) {
    assert(is_valid_color(color));
    assert(castle_side == CASTLE_KING_SIDE || castle_side == CASTLE_QUEEN_SIDE);

    const enum Square king_source      = king_start_square(color);
    const enum Square king_destination = (castle_side == CASTLE_KING_SIDE)
                                       ? square_step(king_source, 2 * DIRECTION_EAST)
                                       : square_step(king_source, 2 * DIRECTION_WEST);

    return new_move(king_source, king_destination, MOVE_TYPE_CASTLE);
}



#endif /* #ifndef WINDMOLEN_MOVE_H_ */
