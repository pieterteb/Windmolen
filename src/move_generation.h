#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H


#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "position.h"
#include "types.h"



#define MAX_MOVES 256


/*
 * We use 16 bits to describe a move. We need 6 bits or the source square as well as the destination square.
 * The remaining 4 bits are used for special moves:
 *
 *      Bits 0-5: source square (0-63),
 *      Bits 6-11: destination square (0-63),
 *      Bits 12-13: special bits (promotion: 1, castle: 2, en passant: 3),
 *      Bits 14-15: promotion piece - 1
 */
typedef uint16_t Move;


typedef Move MoveType;
enum {
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

/* Returns the source square of `move`. */
static inline Square move_source(Move move) {
    return (Square)(move & 0X003F);
}

/* Returns the destination square of `move`. */
static inline Square move_destination(Move move) {
    return (Square)((move >> 6) & 0X003F);
}

/* Returns whether `move` is valid. */
static inline bool is_valid_move(Move move) {
    return move_source(move) != move_destination(move);
}

/* Returns the type of `move`. */
static inline MoveType move_type(Move move) {
    assert(is_valid_move(move));

    return (MoveType)(move & (3 << 12));
}

/* Returns the character corresponding to the promotion type of `move`. Only makes sence if the move is an actual
 * promotion. */
static inline char promotion_to_char(Move move) {
    assert(is_valid_move(move));
    assert(move_type(move) == MOVE_TYPE_PROMOTION);

    static const char promotion_piece_char[4] = {'n', 'b', 'r', 'q'};

    return promotion_piece_char[move >> 14];
}

/* Returns the piece type that corresponds to the promotion type of `move`. */
static inline PieceType promotion_to_piece_type(Move move) {
    assert(is_valid_move(move));
    assert(move_type(move) == MOVE_TYPE_PROMOTION);

    return (PieceType)((move >> 14) + 1);
}


/* Returns a move from `from` to `to`. */
static inline Move new_move(Square from, Square to, MoveType type) {
    assert(is_valid_square(from));
    assert(is_valid_square(to));
    assert(from != to);

    return (Move)(from | (to << 6) | type);
}

/* Adds promotion moves from `from` to `to` to `movelist`. */
static inline Move* new_promotions(Move* movelist, Square from, Square to) {
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


/* Returns a castle move of type `castle_type`. */
static inline Move new_castle(CastlingRights castle_type) {
    assert(castle_type == CASTLE_WHITE_00 || castle_type == CASTLE_WHITE_000 || castle_type == CASTLE_BLACK_00
           || castle_type == CASTLE_BLACK_000);

    const Square king_square    = (castle_type & CASTLE_WHITE) ? SQUARE_E1 : SQUARE_E8;
    const Direction castle_step = (castle_type & CASTLE_KING_SIDE) ? 2 * DIRECTION_EAST : 2 * DIRECTION_WEST;

    return new_move(king_square, king_square + castle_step, MOVE_TYPE_CASTLE);
}


/* Generates all legal moves in `position` to `movelist` and returns the number of legal moves found. */
size_t generate_legal_moves(struct Position* position, Move movelist[256]);

/* Prints `move` to `stdout`. */
void print_move(FILE* stream, Move move);



#endif /* #ifndef MOVE_GENERATION_H */
