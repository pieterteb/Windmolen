#ifndef POSITION_H
#define POSITION_H


#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitboard.h"
#include "types.h"
#include "util.h"



typedef uint16_t Move;


struct Position {
    Bitboard occupancy_by_piece[PIECE_COUNT];
    Bitboard occupancy_by_type[PIECE_TYPE_COUNT - 1]; // We do not differentiate between white and black pawns here.
    Bitboard occupancy_by_color[COLOR_COUNT];
    Bitboard total_occupancy;
    Bitboard checkers[COLOR_COUNT];
    Bitboard blockers[COLOR_COUNT];
    Piece piece_on_square[SQUARE_COUNT];
    Square king_square[COLOR_COUNT];
    Square en_passant_square;

    CastlingRights castling_rights;
    Color side_to_move;

    int halfmove_clock;
    int fullmove_counter;
};

/* Returns the square of the king of `color` in `position`. */
static inline Square king_square(const struct Position* position, Color color) {
    assert(position != NULL);
    assert(is_valid_color(color));
    assert(popcount64(position->occupancy_by_piece[get_piece(color, PIECE_TYPE_KING)]) == 1);

    return position->king_square[color];
}

/* Returns a bitboard of the occupancy of the piece of `piece_type` and `color` in `position`. */
static inline Bitboard piece_occupancy(const struct Position* position, Color color, PieceType piece_type) {
    assert(position != NULL);
    assert(is_valid_color(color));
    assert(is_valid_piece_type(piece_type));

    return position->occupancy_by_piece[get_piece(color, piece_type)];
}

/* Returns a bitboard of the occupancy of `piece` in `position`. */
static inline Bitboard piece_occupancy_by_piece(const struct Position* position, Piece piece) {
    assert(position != NULL);
    assert(is_valid_piece(piece));

    return position->occupancy_by_piece[piece];
}

/*  Returns a bitboard of the occupancy of the pieces of `piece_type` in `position`. */
static inline Bitboard piece_occupancy_by_type(const struct Position* position, PieceType piece_type) {
    assert(position != NULL);
    assert(is_valid_piece_type(piece_type));
    assert(piece_type != PIECE_TYPE_BLACK_PAWN);

    return position->occupancy_by_type[piece_type];
}

/* Returns a bitboard of the occupancy of all pieces of `color` in `position`. */
static inline Bitboard piece_occupancy_by_color(const struct Position* position, Color color) {
    assert(position != NULL);
    assert(is_valid_color(color));

    return position->occupancy_by_color[color];
}

/* Returns a bitboard of the occupancy of bishops and queens of `color` in `position`. */
static inline Bitboard bishop_queen_occupancy(const struct Position* position, Color color) {
    assert(position != NULL);
    assert(is_valid_color(color));

    return piece_occupancy(position, color, PIECE_TYPE_BISHOP) | piece_occupancy(position, color, PIECE_TYPE_QUEEN);
}

/* Returns a bitboard of the occupancy of bishops and queens in `position`. */
static inline Bitboard bishop_queen_occupancy_by_type(const struct Position* position) {
    assert(position != NULL);

    return piece_occupancy_by_type(position, PIECE_TYPE_BISHOP) | piece_occupancy_by_type(position, PIECE_TYPE_QUEEN);
}

/* Returns a bitboard of the occupancy of bishops and queens of `color` in `position`. */
static inline Bitboard rook_queen_occupancy(const struct Position* position, Color color) {
    assert(position != NULL);
    assert(is_valid_color(color));

    return piece_occupancy(position, color, PIECE_TYPE_ROOK) | piece_occupancy(position, color, PIECE_TYPE_QUEEN);
}

/* Returns a bitboard of the occupancy of rooks and queens in `position`. */
static inline Bitboard rook_queen_occupancy_by_type(const struct Position* position) {
    assert(position != NULL);

    return piece_occupancy_by_type(position, PIECE_TYPE_ROOK) | piece_occupancy_by_type(position, PIECE_TYPE_QUEEN);
}

/* Returns which piece is on `square` in `position`. */
static inline Piece piece_on_square(const struct Position* position, Square square) {
    assert(position != NULL);
    assert(is_valid_square(square));

    return position->piece_on_square[square];
}


/* Returns whether `square` is attacked by pieces of `color` with `occupancy` in `position`. */
static inline bool square_is_attacked(const struct Position* position, Color color, Square square, Bitboard occupancy) {
    assert(position != NULL);
    assert(is_valid_color(color));
    assert(is_valid_square(square));

    return ((piece_base_attacks(PIECE_TYPE_BISHOP, square) & bishop_queen_occupancy(position, color)) != EMPTY_BITBOARD
            && (bishop_attacks(square, occupancy) & bishop_queen_occupancy(position, color)) != EMPTY_BITBOARD)
        || ((piece_base_attacks(PIECE_TYPE_ROOK, square) & rook_queen_occupancy(position, color)) != EMPTY_BITBOARD
            && (rook_attacks(square, occupancy) & rook_queen_occupancy(position, color)) != EMPTY_BITBOARD)
        || ((piece_base_attacks(PIECE_TYPE_KNIGHT, square) & piece_occupancy(position, color, PIECE_TYPE_KNIGHT))
            != EMPTY_BITBOARD)
        || ((piece_base_attacks(get_pawn_type(!color), square) & piece_occupancy(position, color, PIECE_TYPE_PAWN))
            != EMPTY_BITBOARD)
        || ((piece_base_attacks(PIECE_TYPE_KING, square) & piece_occupancy(position, color, PIECE_TYPE_KING))
            != EMPTY_BITBOARD);
}

/* Returns a bitboard of the pieces of that attack `square` with `occupancy` in `position`. */
static inline Bitboard attackers_of_square(const struct Position* position, Square square, Bitboard occupancy) {
    assert(position != NULL);
    assert(is_valid_square(square));

    return ((bishop_attacks(square, occupancy) & bishop_queen_occupancy_by_type(position))
            | (rook_attacks(square, occupancy) & rook_queen_occupancy_by_type(position))
            | (piece_base_attacks(PIECE_TYPE_KNIGHT, square) & piece_occupancy_by_type(position, PIECE_TYPE_KNIGHT))
            | (piece_base_attacks(PIECE_TYPE_WHITE_PAWN, square) & piece_occupancy_by_piece(position, PIECE_BLACK_PAWN))
            | (piece_base_attacks(PIECE_TYPE_BLACK_PAWN, square) & piece_occupancy_by_piece(position, PIECE_WHITE_PAWN))
            | (piece_base_attacks(PIECE_TYPE_KING, square) & piece_occupancy_by_type(position, PIECE_TYPE_KING)));
}


void do_move(struct Position* position, Move move);


char* position_to_string(const struct Position* position, size_t* size_out);
struct Position position_from_FEN(const char* fen);
char* position_to_FEN(const struct Position* position, size_t* size_out);



#endif /* POSITION_H */
