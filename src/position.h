#ifndef POSITION_H
#define POSITION_H


#include <ctype.h>

#include "bitboard.h"
#include "types.h"



typedef struct Position {
    Bitboard board[PIECE_COUNT];
    Bitboard occupancy[COLOR_COUNT];
    Bitboard checkers[COLOR_COUNT];
    Bitboard en_passant;

    Color side_to_move;
    CastlingRights castling_rights;
    int halfmove_clock;
    int fullmove_counter;
} Position;


char* position_to_string(Position* position, size_t* size_out);

Position position_from_FEN(const char* fen);
void position_to_FEN(Position* position, char* fen_out);



#endif /* POSITION_H */
