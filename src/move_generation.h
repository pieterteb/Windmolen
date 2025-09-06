#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H


#include <stdint.h>



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

#define MOVE_SOURCE(move)                   ((move) & 0x003f)
#define MOVE_DESTINATION(move)              ((move) & 0x0fc0)

typedef Move MoveType;

#define MOVE_TYPE(move)                     ((move) & (MoveType)0xf000)

#define MOVE_TYPE_QUIET                     ((MoveType)0b0000 << 12U)
#define MOVE_TYPE_KING_CASTLE               ((MoveType)0b1000 << 12U)
#define MOVE_TYPE_QUEEN_CASTLE              ((MoveType)0b1100 << 12U)
#define MOVE_TYPE_CAPTURE                   ((MoveType)0b0001 << 12U)
#define MOVE_TYPE_EN_PASSANT_CAPTURE        ((MoveType)0b0101 << 12U)
#define MOVE_TYPE_DOUBLE_PAWN_PUSH          ((MoveType)0b0100 << 12U)
#define MOVE_TYPE_KNIGHT_PROMOTION          ((MoveType)0b0010 << 12U)
#define MOVE_TYPE_BISHOP_PROMOTION          ((MoveType)0b0110 << 12U)
#define MOVE_TYPE_ROOK_PROMOTION            ((MoveType)0b1010 << 12U)
#define MOVE_TYPE_QUEEN_PROMOTION           ((MoveType)0b1110 << 12U)
#define MOVE_TYPE_KNIGHT_PROMOTION_CAPTURE  (MOVE_TYPE_CAPTURE | MOVE_TYPE_KNIGHT_PROMOTION)
#define MOVE_TYPE_BISHOP_PROMOTION_CAPTURE  (MOVE_TYPE_CAPTURE | MOVE_TYPE_BISHOP_PROMOTION)
#define MOVE_TYPE_ROOK_PROMOTION_CAPTURE    (MOVE_TYPE_CAPTURE | MOVE_TYPE_ROOK_PROMOTION)
#define MOVE_TYPE_QUEEN_PROMOTION_CAPTURE   (MOVE_TYPE_CAPTURE | MOVE_TYPE_QUEEN_PROMOTION)



#endif /* MOVE_GENERATION_H */