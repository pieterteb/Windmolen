#ifndef WINDMOLEN_MOVE_GENERATION_H_
#define WINDMOLEN_MOVE_GENERATION_H_


#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "position.h"
#include "types.h"
#include "util.h"



constexpr size_t MAX_MOVES = 256;  // An upperbound for the maximum number of pseudolegal moves in a chess position.


/*
 * We use 16 bits to describe a move. We need 6 bits for the source square as well as the destination square.
 * The remaining 4 bits are used for special moves:
 *
 *      Bits 0-5: source square (0-63),
 *      Bits 6-11: destination square (0-63),
 *      Bits 12-13: special bits (promotion: 1, castle: 2, en passant: 3),
 *      Bits 14-15: promotion piece - 1
 */
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
static INLINE enum Square move_source(Move move) {
    return move & 0X003F;  // Get first 6 bits.
}

// Returns the destination square of `move`.
static INLINE enum Square move_destination(Move move) {
    return (move >> 6) & 0X003F;  // Get second 6 bits.
}

// Returns whether `move` is potentially valid.
static INLINE bool is_valid_move(Move move) {
    return move_source(move) != move_destination(move);
}

// Returns the type of `move`.
static INLINE enum MoveType move_type(Move move) {
    assert(is_valid_move(move));

    return move & (3 << 12);  // Mask bits 12 and 13.
}

// Returns the character corresponding to the promotion type of `move`. Calling this function only makes sense if the
// move is an actual promotion.
static INLINE char promotion_to_char(Move move) {
    assert(is_valid_move(move));
    assert(move_type(move) == MOVE_TYPE_PROMOTION);

    static const char promotion_piece_char[4] = {'n', 'b', 'r', 'q'};

    return promotion_piece_char[move >> 14];  // Get promotion bits (14 and 15).
}

// Returns the piece type that corresponds to the promotion type of `move`.
static INLINE enum PieceType promotion_piece_type(Move move) {
    assert(is_valid_move(move));
    assert(move_type(move) == MOVE_TYPE_PROMOTION);

    return (enum PieceType)((move >> 14) + 1);
}


// Returns a move from `from` to `to` of `type`.
static INLINE Move new_move(enum Square from, enum Square to, enum MoveType type) {
    assert(is_valid_square(from));
    assert(is_valid_square(to));
    assert(from != to);

    return (Move)(from | (to << 6) | type);
}

// Adds promotion moves from `from` to `to` to `movelist`.
static INLINE Move* new_promotions(Move* movelist, enum Square from, enum Square to) {
    assert(movelist != NULL);
    assert(is_valid_square(from));
    assert(is_valid_square(to));
    assert(from != to);

    *movelist++ = new_move(from, to, MOVE_TYPE_KNIGHT_PROMOTION);
    *movelist++ = new_move(from, to, MOVE_TYPE_BISHOP_PROMOTION);
    *movelist++ = new_move(from, to, MOVE_TYPE_ROOK_PROMOTION);
    *movelist++ = new_move(from, to, MOVE_TYPE_QUEEN_PROMOTION);

    return movelist;
}

// Returns a castle move of type `castle_type`.
static INLINE Move new_castle(enum CastlingRights castle_type) {
    assert(castle_type == CASTLE_WHITE_00 || castle_type == CASTLE_WHITE_000 || castle_type == CASTLE_BLACK_00
           || castle_type == CASTLE_BLACK_000);

    const enum Square king_square    = ((castle_type & CASTLE_WHITE) != CASTLE_NONE) ? SQUARE_E1 : SQUARE_E8;
    const enum Direction castle_step = ((castle_type & CASTLE_KING_SIDE) != CASTLE_NONE) ? 2 * DIRECTION_EAST
                                                                                         : 2 * DIRECTION_WEST;

    return new_move(king_square, king_square + (enum Square)castle_step, MOVE_TYPE_CASTLE);
}


// Generates all legal moves in `position` to `movelist` and returns the number of legal moves found.
size_t generate_legal_moves(struct Position* position, Move movelist[256]);



#endif /* #ifndef WINDMOLEN_MOVE_GENERATION_H_ */
