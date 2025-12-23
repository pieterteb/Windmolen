#ifndef WINDMOLEN_POSITION_H_
#define WINDMOLEN_POSITION_H_


#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "bitboard.h"
#include "move.h"
#include "score.h"
#include "util.h"
#include "zobrist.h"



// Clarification: Currently, we are storing/computing the blockers and king_square fields for both colors at all times.
// In the code however, we only ever use them for the side to move. Still, we keep them as they are likely to be needed
// for future engine enhancements.

// Structure used for undoing moves and detecting threefold repetitions.
struct PositionInfo {
    enum CastlingRights castling_rights;
    enum Square en_passant_square;
    size_t halfmove_clock;

    Score middle_game_score[COLOR_COUNT];
    Score end_game_score[COLOR_COUNT];
    int game_phase;

    struct PositionInfo* previous_info;
    ZobristKey zobrist_key;
    Bitboard checkers;
    Bitboard blockers[COLOR_COUNT];
    enum Piece captured_piece;
    int repetition;
};

// Structure that describes a chess position.
struct Position {
    Bitboard occupancy_by_type[PIECE_TYPE_COUNT - 1];  // We do not differentiate between white and black pawns here.
    Bitboard occupancy_by_color[COLOR_COUNT];
    Bitboard total_occupancy;

    struct PositionInfo* info;

    enum Square king_square[COLOR_COUNT];
    enum Color side_to_move;
    size_t plies_since_start;
    size_t fullmove_counter;

    enum Piece piece_on_square[SQUARE_COUNT];
};


// Returns the Zobrist key of `position`.
static INLINE ZobristKey zobrist_key(const struct Position* position) {
    assert(position != nullptr);

    return position->info->zobrist_key;
}


// Returns which piece is on `square` in `position`.
static INLINE enum Piece piece_on_square(const struct Position* position, const enum Square square) {
    assert(position != nullptr);
    assert(is_valid_square(square));

    return position->piece_on_square[square];
}

// Returns a bitboard of the occupancy of the pieces of `piece_type` in `position`.
static INLINE Bitboard piece_occupancy_by_type(const struct Position* position, const enum PieceType piece_type) {
    assert(position != nullptr);
    assert(is_valid_piece_type(piece_type));
    assert(piece_type != PIECE_TYPE_BLACK_PAWN);

    return position->occupancy_by_type[piece_type];
}

// Returns a bitboard of the occupancy of all pieces of `color` in `position`.
static INLINE Bitboard piece_occupancy_by_color(const struct Position* position, const enum Color color) {
    assert(position != nullptr);
    assert(is_valid_color(color));

    return position->occupancy_by_color[color];
}

// Returns a bitboard of the occupancy of the piece of `color` and `piece_type` in `position`.
static INLINE Bitboard piece_occupancy(const struct Position* position, const enum Color color,
                                       const enum PieceType piece_type) {
    assert(position != nullptr);
    assert(is_valid_color(color));
    assert(is_valid_piece_type(piece_type));

    return piece_occupancy_by_type(position, piece_type) & piece_occupancy_by_color(position, color);
}

// Returns a bitboard of the occupancy of bishops and queens in `position`.
static INLINE Bitboard bishop_queen_occupancy_by_type(const struct Position* position) {
    assert(position != nullptr);

    return piece_occupancy_by_type(position, PIECE_TYPE_BISHOP) | piece_occupancy_by_type(position, PIECE_TYPE_QUEEN);
}

// Returns a bitboard of the occupancy of bishops and queens of `color` in `position`.
static INLINE Bitboard bishop_queen_occupancy(const struct Position* position, const enum Color color) {
    assert(position != nullptr);
    assert(is_valid_color(color));

    return piece_occupancy_by_color(position, color) & bishop_queen_occupancy_by_type(position);
}

// Returns a bitboard of the occupancy of rooks and queens in `position`.
static INLINE Bitboard rook_queen_occupancy_by_type(const struct Position* position) {
    assert(position != nullptr);

    return piece_occupancy_by_type(position, PIECE_TYPE_ROOK) | piece_occupancy_by_type(position, PIECE_TYPE_QUEEN);
}

// Returns a bitboard of the occupancy of rooks and queens of `color` in `position`.
static INLINE Bitboard rook_queen_occupancy(const struct Position* position, const enum Color color) {
    assert(position != nullptr);
    assert(is_valid_color(color));

    return piece_occupancy_by_color(position, color) & rook_queen_occupancy_by_type(position);
}

// Returns the kingsquare of `color` in `position`.
static INLINE enum Square king_square(const struct Position* position, const enum Color color) {
    assert(position != nullptr);
    assert(is_valid_color(color));
    assert(popcount64(piece_occupancy(position, color, PIECE_TYPE_KING)) == 1);

    return position->king_square[color];
}

// Returns a bitboard of the king of `color` in `position`. This function should be slightly faster than
// piece_occupancy(position, color, PIECE_TYPE_KING).
static INLINE Bitboard king_occupancy(const struct Position* position, const enum Color color) {
    assert(position != nullptr);
    assert(is_valid_color(color));

    // This works since there will always be exactly one king per side.
    return square_bitboard(king_square(position, color));
}

// Returns the en passant square of `position`.
static INLINE enum Square en_passant_square(const struct Position* position) {
    assert(position != nullptr);

    return position->info->en_passant_square;
}

// Returns whether the side to move in `position` is in check.
static INLINE bool in_check(const struct Position* position) {
    assert(position != nullptr);

    return position->info->checkers != EMPTY_BITBOARD;
}


// Places `piece` on `square` in `position`.
static INLINE void place_piece(struct Position* position, const enum Piece piece, const enum Square square) {
    assert(position != nullptr);
    assert(is_valid_piece(piece));
    assert(is_valid_square(square));

    const Bitboard bitboard         = square_bitboard(square);
    const enum PieceType piece_type = type_of_piece(piece);
    const enum Color piece_color    = color_of_piece(piece);

    position->piece_on_square[square] = piece;
    position->occupancy_by_type[piece_type] |= bitboard;
    position->occupancy_by_color[piece_color] |= bitboard;

    position->info->middle_game_score[piece_color] += piece_square_value_middle_game[piece][square];
    position->info->end_game_score[piece_color] += piece_square_value_end_game[piece][square];
    position->info->game_phase += game_phase_increment[piece_type];
}

// Removes `piece` from `square` in `position`.
static INLINE void remove_piece(struct Position* position, const enum Square square) {
    assert(position != nullptr);

    const enum Piece piece = piece_on_square(position, square);

    assert(is_valid_piece(piece));
    assert(is_valid_square(square));
    assert(piece_on_square(position, square) == piece);

    const Bitboard bitboard         = square_bitboard(square);
    const enum PieceType piece_type = type_of_piece(piece);
    const enum Color piece_color    = color_of_piece(piece);

    position->piece_on_square[square] = PIECE_NONE;
    position->occupancy_by_type[piece_type] ^= bitboard;
    position->occupancy_by_color[piece_color] ^= bitboard;

    position->info->middle_game_score[piece_color] -= piece_square_value_middle_game[piece][square];
    position->info->end_game_score[piece_color] -= piece_square_value_end_game[piece][square];
    position->info->game_phase -= game_phase_increment[piece_type];
}

// Replaces a piece on `square` with `piece` in `position`.
static INLINE void replace_piece(struct Position* position, const enum Piece piece, const enum Square square) {
    assert(position != nullptr);
    assert(is_valid_piece(piece));
    assert(is_valid_square(square));

    remove_piece(position, square);
    place_piece(position, piece, square);
}

// Moves a piece from `source` to `destination` in `position`.
static INLINE void move_piece(struct Position* position, const enum Square source, const enum Square destination) {
    assert(position != nullptr);
    assert(is_valid_square(source));
    assert(is_valid_square(destination));
    assert(source != destination);

    const Bitboard bitboard      = square_bitboard(source) | square_bitboard(destination);
    const enum Piece piece       = piece_on_square(position, source);
    const enum Color piece_color = color_of_piece(piece);

    position->piece_on_square[source]      = PIECE_NONE;
    position->piece_on_square[destination] = piece;
    position->occupancy_by_type[type_of_piece(piece)] ^= bitboard;
    position->occupancy_by_color[piece_color] ^= bitboard;

    position->info->middle_game_score[piece_color] += (Score)(piece_square_value_middle_game[piece][destination]
                                                              - piece_square_value_middle_game[piece][source]);
    position->info->end_game_score[piece_color] += (Score)(piece_square_value_end_game[piece][destination]
                                                           - piece_square_value_end_game[piece][source]);
    // Game phase does not change.
}


// Returns whether `square` is attacked by pieces of `color` with `occupancy` in `position`.
static INLINE bool square_is_attacked(const struct Position* position, const enum Color color, const enum Square square,
                                      const Bitboard occupancy) {
    assert(position != nullptr);
    assert(is_valid_color(color));
    assert(is_valid_square(square));

    return ((piece_base_attacks(PIECE_TYPE_BISHOP, square) & bishop_queen_occupancy(position, color)) != EMPTY_BITBOARD
            && (bishop_attacks(square, occupancy) & bishop_queen_occupancy(position, color)) != EMPTY_BITBOARD)
        || ((piece_base_attacks(PIECE_TYPE_ROOK, square) & rook_queen_occupancy(position, color)) != EMPTY_BITBOARD
            && (rook_attacks(square, occupancy) & rook_queen_occupancy(position, color)) != EMPTY_BITBOARD)
        || ((piece_base_attacks(PIECE_TYPE_KNIGHT, square) & piece_occupancy(position, color, PIECE_TYPE_KNIGHT))
            != EMPTY_BITBOARD)
        || ((piece_base_attacks(pawn_type_from_color(opposite_color(color)), square)
             & piece_occupancy(position, color, PIECE_TYPE_PAWN))
            != EMPTY_BITBOARD)
        || ((piece_base_attacks(PIECE_TYPE_KING, square) & king_occupancy(position, color)) != EMPTY_BITBOARD);
}

// Returns a bitboard of the pieces that attack `square` with `occupancy` in `position`.
static INLINE Bitboard attackers_of_square(const struct Position* position, const enum Square square,
                                           const Bitboard occupancy) {
    assert(position != nullptr);
    assert(is_valid_square(square));

    return (bishop_attacks(square, occupancy) & bishop_queen_occupancy_by_type(position))
         | (rook_attacks(square, occupancy) & rook_queen_occupancy_by_type(position))
         | (piece_base_attacks(PIECE_TYPE_KNIGHT, square) & piece_occupancy_by_type(position, PIECE_TYPE_KNIGHT))
         | (piece_base_attacks(PIECE_TYPE_WHITE_PAWN, square) & piece_occupancy(position, COLOR_BLACK, PIECE_TYPE_PAWN))
         | (piece_base_attacks(PIECE_TYPE_BLACK_PAWN, square) & piece_occupancy(position, COLOR_WHITE, PIECE_TYPE_PAWN))
         | (piece_base_attacks(PIECE_TYPE_KING, square) & piece_occupancy_by_type(position, PIECE_TYPE_KING));
}


// Returns whether castling king side for white is unobstructed in `position`.
static INLINE bool white_king_side_unobstructed(const struct Position* position) {
    assert(position != nullptr);

    return (between_bitboard(SQUARE_E1, SQUARE_G1) & position->total_occupancy) == EMPTY_BITBOARD;
}

// Returns whether castling queen side for white is unobstructed in `position`.
static INLINE bool white_queen_side_unobstructed(const struct Position* position) {
    assert(position != nullptr);

    return (between_bitboard(SQUARE_E1, SQUARE_B1) & position->total_occupancy) == EMPTY_BITBOARD;
}

// Returns whether castling king side for black is unobstructed in `position`.
static INLINE bool black_king_side_unobstructed(const struct Position* position) {
    assert(position != nullptr);

    return (between_bitboard(SQUARE_E8, SQUARE_G8) & position->total_occupancy) == EMPTY_BITBOARD;
}

// Returns whether castling queen side for black is unobstructed in `position`.
static INLINE bool black_queen_side_unobstructed(const struct Position* position) {
    assert(position != nullptr);

    return (between_bitboard(SQUARE_E8, SQUARE_B8) & position->total_occupancy) == EMPTY_BITBOARD;
}


// Returns `true` if a threefold repetition has occured or if the position has repeated since the start of the search.
static INLINE bool is_repetition(const struct Position* position, const size_t ply) {
    assert(position != nullptr);
    assert(ply > 0);

    // If a threefold has occured, repetition will be negative, so the second inequality will always return true.
    return position->info->repetition != 0 && position->info->repetition < (int)ply;
}

// Returns whether `position` is a draw by repetition or 50-move-rule. We assume that it is not checkmate here.
static INLINE bool is_draw(const struct Position* position, const size_t ply) {
    assert(position != nullptr);
    assert(ply > 0);
    assert(position->info->halfmove_clock <= 100);

    return position->info->halfmove_clock == 100 || is_repetition(position, ply);
}


// Returns whether `move` is a capture in `position`.
static INLINE bool is_capture(const struct Position* position, const Move move) {
    assert(position != nullptr);
    assert(!is_weird_move(move));

    return piece_on_square(position, move_destination(move)) != PIECE_NONE
        || type_of_move(move) == MOVE_TYPE_EN_PASSANT;
}

// Returns whether `move` is irreversible in `position`.
static INLINE bool is_irreversible(const struct Position* position, const Move move) {
    assert(position != nullptr);
    assert(!is_weird_move(move));

    return piece_on_square(position, move_destination(move)) != PIECE_NONE
        || type_of_piece(piece_on_square(position, move_source(move))) == PIECE_TYPE_PAWN;
}

// Returns whether `move` is a direct check in `position`, i.e. a move such that the moved piece attacks the enemy king.
static INLINE bool gives_direct_check(const struct Position* position, const Move move) {
    assert(position != nullptr);
    assert(!is_weird_move(move));

    const enum Color opponent     = opposite_color(position->side_to_move);
    const enum MoveType move_type = type_of_move(move);
    enum PieceType piece_type     = type_of_piece(piece_on_square(position, move_source(move)));
    enum Square destination       = move_destination(move);

    Bitboard occupancy = position->total_occupancy;
    if (move_type == MOVE_TYPE_CASTLE) {
        // Squares where rook ends up after travelling based on king destination square.
        // clang-format off
        static enum Square rook_destinations[SQUARE_COUNT] = {
            [SQUARE_G1] = SQUARE_F1,
            [SQUARE_C1] = SQUARE_D1,
            [SQUARE_G8] = SQUARE_F8,
            [SQUARE_C8] = SQUARE_D8
        };
        // clang-format on

        // The relevant destination is that of the rook. Also move the king out of the way.
        piece_type  = PIECE_TYPE_ROOK;
        destination = rook_destinations[destination];
        occupancy ^= square_bitboard(move_source(move));
    } else if (move_type == MOVE_TYPE_PROMOTION) {
        // The relevant piece type is that of the promotion piece. We also need to remove the pawn in case the king was
        // behind the pawn and we promote to a rook or queen.
        piece_type = promotion_piece_type(move);
        occupancy ^= square_bitboard(move_source(move));
    } else if (piece_type == PIECE_TYPE_PAWN) {
        piece_type = pawn_type_from_color(opponent);
    }

    return (piece_attacks(piece_type, king_square(position, opponent), occupancy) & square_bitboard(destination))
        != EMPTY_BITBOARD;
}

// Returns whether `move` is a discovered check in `position`, i.e. a move such that the piece moving reveals an attack
// on the king.
static INLINE bool gives_discovered_check(const struct Position* position, const Move move) {
    assert(position != nullptr);
    assert(!is_weird_move(move));

    const enum Color side_to_move = position->side_to_move;
    const enum Color opponent     = opposite_color(side_to_move);

    // If the move is not en passant, it is a discovered check if and only if the moved piece is a blocker and is
    // moved in a different direction than on the line formed by the king and the attacker.
    const bool is_blocker = (position->info->blockers[opponent] & square_bitboard(move_source(move))) != EMPTY_BITBOARD;
    if (is_blocker) {
        const bool is_discovery = (line_bitboard(move_source(move), move_destination(move))
                                   & king_occupancy(position, opponent))
                               == EMPTY_BITBOARD;

        if (is_discovery)
            return true;
    }

    // If the above did not return true, there is still the possibility that we have an en passant move where an
    // attack on the king is created after capturing the pawn.
    if (type_of_move(move) == MOVE_TYPE_EN_PASSANT) {
        // We essentially perform the capture on the position and make sure a potential checker is not the pawn that was
        // just moved, i.e. the move is not a direct check. For this, we only remove the moved pawn and the capture pawn
        // from the total occupancy, and then check if the king is attacked.
        const Bitboard source_bitboard      = square_bitboard(move_source(move));
        const Bitboard destination_bitboard = square_bitboard(move_destination(move));
        const Bitboard captured_bitboard    = (opponent == COLOR_BLACK) ? shift_bitboard_south(destination_bitboard)
                                                                        : shift_bitboard_north(destination_bitboard);

        const Bitboard occupancy = (position->total_occupancy | destination_bitboard) ^ source_bitboard
                                 ^ captured_bitboard;
        const enum Square king = king_square(position, opponent);

        // We manually inline attackers_of_square() here and modify it. It is guaranteed that the king is not attacked
        // by one of our pawns, knights or our king.
        const bool king_is_attacked = (bishop_attacks(king, occupancy) & bishop_queen_occupancy(position, side_to_move))
                                   != EMPTY_BITBOARD
                                   || (rook_attacks(king, occupancy) & rook_queen_occupancy(position, side_to_move))
                                      != EMPTY_BITBOARD;

        return king_is_attacked;
    }

    return false;
}


// Performs `move` on `position`. We assume that a legal move is supplied.
void do_move(struct Position* position, struct PositionInfo* new_info, const Move move);

// Reverts `position` to the position before `move` was made.
void undo_move(struct Position* position, const Move move);


// Sets `position` to the start position of chess.
void setup_start_position(struct Position* position, struct PositionInfo* info);

// Sets `position` to the kiwipete position (https://www.chessprogramming.org/Perft_Results#Position_2).
void setup_kiwipete_position(struct Position* position, struct PositionInfo* info);

// Sets `position` from Forsyth-Edwards Notation (FEN)
// (https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation).
const char* setup_position_from_fen(struct Position* position, struct PositionInfo* info, const char* fen);


// Prints FEN of `position` to `stdout`.
void print_fen(const struct Position* position);

// Prints `position` to `stdout`.
void print_position(const struct Position* position);



#endif /* #ifndef WINDMOLEN_POSITION_H_ */
