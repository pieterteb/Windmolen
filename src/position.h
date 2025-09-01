#ifndef POSITION_H
#define POSITION_H


#include "bitboard.h"
#include "constants.h"



typedef struct Position {
    Bitboard board[PIECE_COUNT];
    int side_to_move;
    int castling_rights;
    Bitboard en_passant;
    int halfmove_clock;
    int fullmove_counter;
} Position;


Position position_from_FEN(const char* fen);
void position_to_FEN(Position* position, char* fen_out);



#endif /* POSITION_H */
