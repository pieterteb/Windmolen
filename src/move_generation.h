#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H


#include <stdint.h>

#include "types.h"



#define MAX_MOVES   256


/*
 * We use 16 bits to describe a move. We need 6 bits or the source square as well as the destination square. 
 * The remaining 4 bits are used for special moves:
 *
 *      Bits 0-5: source square,
 *      Bits 6-11: destination square,
 *      Bit 12: capture flag,
 *      Bit 13: promotion flag,
 *      Bit 14: special 0,
 *      Bit 15: special 1.
 * 
 */
typedef uint16_t Move;

typedef Move MoveType;
enum MoveType {
    MOVE_TYPE_QUIET =                       0b0000 << 12,
    MOVE_TYPE_KING_CASTLE =                 0b1000 << 12,
    MOVE_TYPE_QUEEN_CASTLE =                0b1100 << 12,
    MOVE_TYPE_CAPTURE =                     0b0001 << 12,
    MOVE_TYPE_EN_PASSANT_CAPTURE =          0b0101 << 12,
    MOVE_TYPE_DOUBLE_PAWN_PUSH =            0b0100 << 12,
    MOVE_TYPE_KNIGHT_PROMOTION =            0b0010 << 12,
    MOVE_TYPE_BISHOP_PROMOTION =            0b0110 << 12,
    MOVE_TYPE_ROOK_PROMOTION =              0b1010 << 12,
    MOVE_TYPE_QUEEN_PROMOTION =             0b1110 << 12,
    MOVE_TYPE_KNIGHT_PROMOTION_CAPTURE =    MOVE_TYPE_CAPTURE | MOVE_TYPE_KNIGHT_PROMOTION,
    MOVE_TYPE_BISHOP_PROMOTION_CAPTURE =    MOVE_TYPE_CAPTURE | MOVE_TYPE_BISHOP_PROMOTION,
    MOVE_TYPE_ROOK_PROMOTION_CAPTURE =      MOVE_TYPE_CAPTURE | MOVE_TYPE_ROOK_PROMOTION,
    MOVE_TYPE_QUEEN_PROMOTION_CAPTURE =     MOVE_TYPE_CAPTURE | MOVE_TYPE_QUEEN_PROMOTION,
};

static inline Square move_source(Move move) {
    return (Square)(move & 0x003f);
}

static inline Square move_destination(Move move) {
    return (Square)((move & 0x0fc0) >> 6);
}



#endif /* MOVE_GENERATION_H */