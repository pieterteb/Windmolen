#ifndef POSITION_H
#define POSITION_H


#include <assert.h>
#include <ctype.h>

#include "bitboard.h"
#include "types.h"
#include "util.h"

typedef uint16_t Move;


typedef struct Position {
    Bitboard board[PIECE_COUNT];
    Bitboard occupancy[COLOR_COUNT];
    Bitboard total_occupancy;
    Bitboard checkers[COLOR_COUNT];
    Bitboard blockers[COLOR_COUNT];
    Piece pieces[SQUARE_COUNT];
    Square en_passant_square;

    Bitboard castling_squares[CASTLING_COUNT];

    Color side_to_move;
    CastlingRights castling_rights;
    int halfmove_clock;
    int fullmove_counter;
} Position;

static inline Square get_king_square(const struct Position* position) {
    assert(position != NULL);

    return (Square)lsb64(position->board[get_piece(position->side_to_move, PIECE_TYPE_KING)]);
}


static inline Bitboard get_bishop_queen_occupancy(const struct Position* position, Color color) {
    assert(position != NULL);
    assert(is_valid_color(color));

    return position->board[get_piece(color, PIECE_TYPE_BISHOP)] | position->board[get_piece(color, PIECE_TYPE_QUEEN)];
}

static inline Bitboard get_rook_queen_occupancy(const struct Position* position, Color color) {
    assert(position != NULL);
    assert(is_valid_color(color));

    return position->board[get_piece(color, PIECE_TYPE_ROOK)] | position->board[get_piece(color, PIECE_TYPE_QUEEN)];
}


void do_move(struct Position* position, Move move);

bool is_legal_pinned_move(const struct Position* position, Move move);
bool is_legal_king_move(const struct Position* position, Move move);

Bitboard compute_checkers(const struct Position* position, Color color);
Bitboard compute_blockers(const struct Position* position, Color color);


char* position_to_string(Position* position, size_t* size_out);

Position position_from_FEN(const char* fen);
char* position_to_FEN(Position* position, size_t* size_out);



#endif /* POSITION_H */
