#ifndef WINDMOLEN_POSITION_H_
#define WINDMOLEN_POSITION_H_


#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "bitboard.h"
#include "types.h"
#include "util.h"
#include "zobrist.h"



typedef uint16_t Move;


struct Position {
    ZobristHash zobrist_hash;
    ZobristHash repetition_hashes[99];
    size_t repetition_hash_count;

    Bitboard occupancy_by_type[PIECE_TYPE_COUNT - 1];  // We do not differentiate between white and black pawns here.
    Bitboard occupancy_by_color[COLOR_COUNT];
    Bitboard total_occupancy;
    Bitboard checkers[COLOR_COUNT];
    Bitboard blockers[COLOR_COUNT];
    Square king_square[COLOR_COUNT];
    Square en_passant_square;

    CastlingRights castling_rights;
    Color side_to_move;

    size_t halfmove_clock;
    size_t fullmove_counter;

    Piece piece_on_square[SQUARE_COUNT];
};


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

/* Returns a bitboard of the occupancy of the piece of `piece_type` and `color` in `position`. */
static inline Bitboard piece_occupancy(const struct Position* position, Color color, PieceType piece_type) {
    assert(position != NULL);
    assert(is_valid_color(color));
    assert(is_valid_piece_type(piece_type));

    return piece_occupancy_by_type(position, piece_type) & piece_occupancy_by_color(position, color);
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


/* Returns the square of the king of `color` in `position`. */
static inline Square king_square(const struct Position* position, Color color) {
    assert(position != NULL);
    assert(is_valid_color(color));
    assert(popcount64(piece_occupancy_by_type(position, PIECE_TYPE_KING)) == 2);

    return position->king_square[color];
}

static inline bool in_check(const struct Position* position) {
    assert(position != NULL);

    return position->checkers[position->side_to_move] != EMPTY_BITBOARD;
}


static inline void place_piece(struct Position* position, Piece piece, Square square) {
    assert(position != NULL);
    assert(is_valid_piece(piece));
    assert(is_valid_square(square));

    Bitboard bitboard = square_bitboard(square);
    position->occupancy_by_type[type_of_piece(piece)] |= bitboard;
    position->occupancy_by_color[color_of_piece(piece)] |= bitboard;
    position->piece_on_square[square] = piece;
    position->zobrist_hash ^= piece_zobrist_keys[piece][square];
}

static inline void place_piece_type(struct Position* position, Color color, PieceType piece_type, Square square) {
    assert(position != NULL);
    assert(is_valid_color(color));
    assert(is_valid_piece_type(piece_type));
    assert(is_valid_square(square));

    Piece piece       = create_piece(color, piece_type);
    Bitboard bitboard = square_bitboard(square);
    position->occupancy_by_type[piece_type] |= bitboard;
    position->occupancy_by_color[color] |= bitboard;
    position->piece_on_square[square] = piece;
    position->zobrist_hash ^= piece_zobrist_keys[piece][square];
}

static inline void remove_piece(struct Position* position, Piece piece, Square square) {
    assert(position != NULL);
    assert(is_valid_piece(piece));
    assert(is_valid_square(square));

    Bitboard bitboard = square_bitboard(square);

    assert((piece_occupancy_by_type(position, type_of_piece(piece)) & bitboard) != 0);
    assert((piece_occupancy_by_color(position, color_of_piece(piece)) & bitboard) != 0);
    assert(piece_on_square(position, square) == piece);

    position->occupancy_by_type[type_of_piece(piece)] ^= bitboard;
    position->occupancy_by_color[color_of_piece(piece)] ^= bitboard;
    position->piece_on_square[square] = PIECE_NONE;
    position->zobrist_hash ^= piece_zobrist_keys[piece][square];
}

static inline void remove_piece_type(struct Position* position, Color color, PieceType piece_type, Square square) {
    assert(position != NULL);
    assert(is_valid_color(color));
    assert(is_valid_piece_type(piece_type));
    assert(is_valid_square(square));

    Bitboard bitboard = square_bitboard(square);

    assert((piece_occupancy_by_type(position, piece_type) & bitboard) != 0);
    assert((piece_occupancy_by_color(position, color) & bitboard) != 0);
    assert(piece_on_square(position, square) == create_piece(color, piece_type));

    position->occupancy_by_type[piece_type] ^= bitboard;
    position->occupancy_by_color[color] ^= bitboard;
    position->piece_on_square[square] = PIECE_NONE;
    position->zobrist_hash ^= piece_zobrist_keys[create_piece(color, piece_type)][square];
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
        || ((piece_base_attacks(type_of_pawn(!color), square) & piece_occupancy(position, color, PIECE_TYPE_PAWN))
            != EMPTY_BITBOARD)
        || ((piece_base_attacks(PIECE_TYPE_KING, square) & piece_occupancy(position, color, PIECE_TYPE_KING))
            != EMPTY_BITBOARD);
}

/* Returns a bitboard of the pieces of that attack `square` with `occupancy` in `position`. */
static inline Bitboard attackers_of_square(const struct Position* position, Square square, Bitboard occupancy) {
    assert(position != NULL);
    assert(is_valid_square(square));

    return (
    (bishop_attacks(square, occupancy) & bishop_queen_occupancy_by_type(position))
    | (rook_attacks(square, occupancy) & rook_queen_occupancy_by_type(position))
    | (piece_base_attacks(PIECE_TYPE_KNIGHT, square) & piece_occupancy_by_type(position, PIECE_TYPE_KNIGHT))
    | (piece_base_attacks(PIECE_TYPE_WHITE_PAWN, square) & piece_occupancy(position, COLOR_BLACK, PIECE_TYPE_PAWN))
    | (piece_base_attacks(PIECE_TYPE_BLACK_PAWN, square) & piece_occupancy(position, COLOR_WHITE, PIECE_TYPE_PAWN))
    | (piece_base_attacks(PIECE_TYPE_KING, square) & piece_occupancy_by_type(position, PIECE_TYPE_KING)));
}


static inline bool is_repetition(const struct Position* position) {
    assert(position != NULL);

    const ZobristHash current_hash = position->zobrist_hash;
    size_t count                   = 0;

    for (int i = (int)position->repetition_hash_count - 2; i >= 0; i -= 2) {
        if (position->repetition_hashes[i] == current_hash) {
            ++count;
            if (count == 2)
                return true;
        }
    }

    return false;
}

// We assume that it is not checkmate here.
static inline bool is_draw(const struct Position* position) {
    assert(position != NULL);
    assert(position->halfmove_clock <= 100);

    return position->halfmove_clock == 100 || is_repetition(position);
}


void do_move(struct Position* position, Move move);


void setup_start_position(struct Position* position);
void setup_kiwipete_position(struct Position* position);
const char* setup_position_from_fen(struct Position* position, const char* fen);

void print_fen(const struct Position* position);
void print_position(const struct Position* position);



#endif /* #ifndef WINDMOLEN_POSITION_H_ */
