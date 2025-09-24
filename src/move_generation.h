#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H


#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "position.h"
#include "types.h"



#define MAX_MOVES   256


/*
 * We use 16 bits to describe a move. We need 6 bits or the source square as well as the destination square. 
 * The remaining 4 bits are used for special moves:
 *
 *      Bits 0-5: source square (0-63),
 *      Bits 6-11: destination square (0-63),
 *      Bits 12-13: promotion piece - 2,
 *      Bits 14-15: special bits (promotion: 1, castle: 2, en passant: 3)
 */
typedef uint16_t Move;

typedef Move MoveType;
enum MoveType {
    MOVE_TYPE_NORMAL            =   0 << 14,
    MOVE_TYPE_PROMOTION         =   1 << 14,
    MOVE_TYPE_CASTLE            =   2 << 14,
    MOVE_TYPE_EN_PASSANT        =   3 << 14,
    MOVE_TYPE_KNIGHT_PROMOTION  =   MOVE_TYPE_PROMOTION | (0 << 12),
    MOVE_TYPE_BISHOP_PROMOTION  =   MOVE_TYPE_PROMOTION | (1 << 12),
    MOVE_TYPE_ROOK_PROMOTION    =   MOVE_TYPE_PROMOTION | (2 << 12),
    MOVE_TYPE_QUEEN_PROMOTION   =   MOVE_TYPE_PROMOTION | (3 << 12),
};

static inline Square move_source(Move move) {
    return (Square)(move & 0x003f);
}

static inline Square move_destination(Move move) {
    return (Square)((move & 0x0fc0) >> 6);
}


// Move from from to to of type type.
static inline Move new_move(Square from, Square to, MoveType type) {
    assert(is_valid_square(from) && is_valid_square(to));

    return (Move)(from | (to << 6) | type);
}

// Add promotion moves from from to to to movelist.
static inline Move* new_promotions(Move* movelist, Square from, Square to) {
    assert(movelist != NULL && is_valid_square(from) && is_valid_square(to));

    *movelist++ = new_move(from, to, MOVE_TYPE_KNIGHT_PROMOTION);
    *movelist++ = new_move(from, to, MOVE_TYPE_BISHOP_PROMOTION);
    *movelist++ = new_move(from, to, MOVE_TYPE_ROOK_PROMOTION);
    *movelist++ = new_move(from, to, MOVE_TYPE_KNIGHT_PROMOTION);

    return movelist;
}



size_t generate_all_pseudo_moves(const Position* position, Move* movelist);



#endif /* MOVE_GENERATION_H */
