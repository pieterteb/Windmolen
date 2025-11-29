#ifndef WINDMOLEN_POSITION_H_
#define WINDMOLEN_POSITION_H_


#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include "bitboard.h"
#include "types.h"
#include "util.h"
#include "zobrist.h"



typedef uint16_t Move;  // Redefinition of Move type to prevent circular includes.


struct Position {
    ZobristHash zobrist_hash;
    ZobristHash repetition_hashes[99];
    size_t repetition_hash_count;

    Bitboard occupancy_by_type[PIECE_TYPE_COUNT - 1];  // We do not differentiate between white and black pawns here.
    Bitboard occupancy_by_color[COLOR_COUNT];
    Bitboard total_occupancy;
    Bitboard checkers[COLOR_COUNT];
    Bitboard blockers[COLOR_COUNT];
    enum Square king_square[COLOR_COUNT];
    enum Square en_passant_square;

    enum CastlingRights castling_rights;
    enum Color side_to_move;

    size_t halfmove_clock;
    size_t fullmove_counter;

    enum Piece piece_on_square[SQUARE_COUNT];
};


// Returns a bitboard of the occupancy of the pieces of `piece_type` in `position`.
static INLINE Bitboard piece_occupancy_by_type(const struct Position* position, enum PieceType piece_type) {
    assert(position != nullptr);
    assert(is_valid_piece_type(piece_type));
    assert(piece_type != PIECE_TYPE_BLACK_PAWN);

    return position->occupancy_by_type[piece_type];
}

// Returns a bitboard of the occupancy of all pieces of `color` in `position`.
static INLINE Bitboard piece_occupancy_by_color(const struct Position* position, enum Color color) {
    assert(position != nullptr);
    assert(is_valid_color(color));

    return position->occupancy_by_color[color];
}

// Returns a bitboard of the occupancy of the piece of `color` and `piece_type` in `position`.
static INLINE Bitboard piece_occupancy(const struct Position* position, enum Color color, enum PieceType piece_type) {
    assert(position != nullptr);
    assert(is_valid_color(color));
    assert(is_valid_piece_type(piece_type));

    return piece_occupancy_by_type(position, piece_type) & piece_occupancy_by_color(position, color);
}

// Returns a bitboard of the occupancy of bishops and queens of `color` in `position`.
static INLINE Bitboard bishop_queen_occupancy(const struct Position* position, enum Color color) {
    assert(position != nullptr);
    assert(is_valid_color(color));

    return piece_occupancy(position, color, PIECE_TYPE_BISHOP) | piece_occupancy(position, color, PIECE_TYPE_QUEEN);
}

// Returns a bitboard of the occupancy of bishops and queens in `position`.
static INLINE Bitboard bishop_queen_occupancy_by_type(const struct Position* position) {
    assert(position != nullptr);

    return piece_occupancy_by_type(position, PIECE_TYPE_BISHOP) | piece_occupancy_by_type(position, PIECE_TYPE_QUEEN);
}

// Returns a bitboard of the occupancy of rooks and queens of `color` in `position`.
static INLINE Bitboard rook_queen_occupancy(const struct Position* position, enum Color color) {
    assert(position != nullptr);
    assert(is_valid_color(color));

    return piece_occupancy(position, color, PIECE_TYPE_ROOK) | piece_occupancy(position, color, PIECE_TYPE_QUEEN);
}

// Returns a bitboard of the occupancy of rooks and queens in `position`.
static INLINE Bitboard rook_queen_occupancy_by_type(const struct Position* position) {
    assert(position != nullptr);

    return piece_occupancy_by_type(position, PIECE_TYPE_ROOK) | piece_occupancy_by_type(position, PIECE_TYPE_QUEEN);
}

// Returns which piece is on `square` in `position`.
static INLINE enum Piece piece_on_square(const struct Position* position, enum Square square) {
    assert(position != nullptr);
    assert(is_valid_square(square));

    return position->piece_on_square[square];
}


// Returns the kingsquare of `color` in `position`.
static INLINE enum Square king_square(const struct Position* position, enum Color color) {
    assert(position != nullptr);
    assert(is_valid_color(color));
    assert(popcount64(piece_occupancy_by_type(position, PIECE_TYPE_KING)) == 2);

    return position->king_square[color];
}

// Returns whether the side to move in `position` is in check.
static INLINE bool in_check(const struct Position* position) {
    assert(position != nullptr);

    return position->checkers[position->side_to_move] != EMPTY_BITBOARD;
}


// Places `piece` on `square` in `position`.
static INLINE void place_piece(struct Position* position, enum Piece piece, enum Square square) {
    assert(position != nullptr);
    assert(is_valid_piece(piece));
    assert(is_valid_square(square));

    Bitboard bitboard = square_bitboard(square);
    position->occupancy_by_type[type_of_piece(piece)] |= bitboard;
    position->occupancy_by_color[color_of_piece(piece)] |= bitboard;
    position->piece_on_square[square] = piece;
    position->zobrist_hash ^= piece_zobrist_keys[piece][square];
}

// Places piece of `color` and `piece_type` on `square` in `position`.
static INLINE void place_piece_type(struct Position* position, enum Color color, enum PieceType piece_type, enum Square square) {
    assert(position != nullptr);
    assert(is_valid_color(color));
    assert(is_valid_piece_type(piece_type));
    assert(is_valid_square(square));

    enum Piece piece       = create_piece(color, piece_type);
    Bitboard bitboard = square_bitboard(square);
    position->occupancy_by_type[piece_type] |= bitboard;
    position->occupancy_by_color[color] |= bitboard;
    position->piece_on_square[square] = piece;
    position->zobrist_hash ^= piece_zobrist_keys[piece][square];
}

// Removes `piece` from `square` in `position`.
static INLINE void remove_piece(struct Position* position, enum Piece piece, enum Square square) {
    assert(position != nullptr);
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

// Removes piece of `color` and `piece_type` from `square` in `position`.
static INLINE void remove_piece_type(struct Position* position, enum Color color, enum PieceType piece_type, enum Square square) {
    assert(position != nullptr);
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
// Returns whether `square` is attacked by pieces of `color` with `occupancy` in `position`.
static INLINE bool square_is_attacked(const struct Position* position, enum Color color, enum Square square, Bitboard occupancy) {
    assert(position != nullptr);
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

// Returns a bitboard of the pieces that attack `square` with `occupancy` in `position`.
static INLINE Bitboard attackers_of_square(const struct Position* position, enum Square square, Bitboard occupancy) {
    assert(position != nullptr);
    assert(is_valid_square(square));

    return (
    (bishop_attacks(square, occupancy) & bishop_queen_occupancy_by_type(position))
    | (rook_attacks(square, occupancy) & rook_queen_occupancy_by_type(position))
    | (piece_base_attacks(PIECE_TYPE_KNIGHT, square) & piece_occupancy_by_type(position, PIECE_TYPE_KNIGHT))
    | (piece_base_attacks(PIECE_TYPE_WHITE_PAWN, square) & piece_occupancy(position, COLOR_BLACK, PIECE_TYPE_PAWN))
    | (piece_base_attacks(PIECE_TYPE_BLACK_PAWN, square) & piece_occupancy(position, COLOR_WHITE, PIECE_TYPE_PAWN))
    | (piece_base_attacks(PIECE_TYPE_KING, square) & piece_occupancy_by_type(position, PIECE_TYPE_KING)));
}


// Returns whether `position` has occured for the third time, implying a draw by repetition.
static INLINE bool is_repetition(const struct Position* position) {
    assert(position != nullptr);

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

// Returns whether `position` is a draw by repetition or 50-move-rule. We assume that it is not checkmate here.
static INLINE bool is_draw(const struct Position* position) {
    assert(position != nullptr);
    assert(position->halfmove_clock <= 100);

    return position->halfmove_clock == 100 || is_repetition(position);
}


// Performs `move` on `position`.
void do_move(struct Position* position, Move move);


// Sets `position` to the start position of chess.
void setup_start_position(struct Position* position);

// Sets `position` to the kiwipete position (https://www.chessprogramming.org/Perft_Results#Position_2).
void setup_kiwipete_position(struct Position* position);

// Sets `position` from Forsyth-Edwards Notation (https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation).
const char* setup_position_from_fen(struct Position* position, const char* fen);


// Prints FEN of `positionf` to `stdout`.
void print_fen(const struct Position* position);

// Prints `position` to `stdout`.
void print_position(const struct Position* position);



#endif /* #ifndef WINDMOLEN_POSITION_H_ */
