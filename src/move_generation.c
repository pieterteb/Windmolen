#include "move_generation.h"

#include <assert.h>

#include "bitboard.h"
#include "board.h"
#include "constants.h"
#include "move.h"
#include "piece.h"
#include "position.h"
#include "util.h"



// Adds all different moves from `source` to a square of the `attacks` bitboard to `movelist`, and returns the end of
// the array.
static INLINE Move* splat_piece_moves(Move* movelist, Bitboard attacks, const enum Square source) {
    assert(movelist != nullptr);
    assert(is_valid_square(source));

    // pop_lsb64() returns the index of the least significant bit, in other words, the square index of that bit.
    while (attacks != EMPTY_BITBOARD)
        *movelist++ = new_normal_move(source, (enum Square)pop_lsb64(&attacks));

    return movelist;
}

// Adds all different pawn moves that end on a square of the `pawn_moves` bitboard after moving `direction`, and returns
// the end of the array.
static INLINE Move* splat_pawn_moves(Move* movelist, Bitboard pawn_moves, const enum Direction direction) {
    assert(movelist != nullptr);

    while (pawn_moves != EMPTY_BITBOARD) {
        // pop_lsb64() returns the index of the least significant bit, in other words, the square index of that bit.
        enum Square destination = (enum Square)pop_lsb64(&pawn_moves);
        *movelist++             = new_normal_move(square_step(destination, (enum Direction)(-direction)), destination);
    }

    return movelist;
}

// Adds promotion moves from `source` to `destination` to `movelist`, assuming `source` and `destination` are different
// valid squares, and moves `movelist` to the end of the array.
static INLINE Move* new_promotions(Move* movelist, const enum Square source, const enum Square destination) {
    assert(movelist != nullptr);
    assert(is_valid_square(source));
    assert(is_valid_square(destination));
    assert(source != destination);

    *movelist++ = new_move(source, destination, MOVE_TYPE_KNIGHT_PROMOTION);
    *movelist++ = new_move(source, destination, MOVE_TYPE_BISHOP_PROMOTION);
    *movelist++ = new_move(source, destination, MOVE_TYPE_ROOK_PROMOTION);
    *movelist++ = new_move(source, destination, MOVE_TYPE_QUEEN_PROMOTION);

    return movelist;
}


// Computes all pseudolegal moves for white in `position`, stores them in `movelist` and returns the end of the array. A
// pseudolegal move here, is defined as a normal move that may or may not put the side to move in check. So in order to
// filter out the legal moves, we would have to check that a non-king piece does not reveal an attack on the king or
// that the king destination square is not attacked.
static Move* white_pseudolegal_moves(const struct Position* position, Move movelist[static MAX_MOVES]) {
    assert(position != nullptr);
    assert(movelist != nullptr);
    assert(position->side_to_move == COLOR_WHITE);

    constexpr enum Color SIDE_TO_MOVE = COLOR_WHITE;
    constexpr enum Color OPPONENT     = COLOR_BLACK;

    // All squares that do not contain one of our own pieces are potential targets.
    Bitboard target = ~position->occupancy_by_color[SIDE_TO_MOVE];


    /* Regular king moves. */
    const enum Square king_source = king_square(position, SIDE_TO_MOVE);
    movelist = splat_piece_moves(movelist, piece_base_attacks(PIECE_TYPE_KING, king_source) & target, king_source);

    const Bitboard checkers = position->info->checkers;

    // If we are in double check, only non-castling king moves can get us out of check.
    if (popcount64_greater_than_one(checkers))
        return movelist;  // Immediately return as there are no other possible moves.

    // If not in check, castling moves should be generated. Else, we update the target such that each move will either
    // interpose the check or capture the checker (we have already computed king moves).
    if (checkers == EMPTY_BITBOARD) {
        /* Castling moves. */
        const enum CastlingRights castling_rights = position->info->castling_rights & CASTLE_WHITE;
        if (castling_rights != CASTLE_NONE) {
            if ((castling_rights & CASTLE_KING_SIDE) != CASTLE_NONE && white_king_side_unobstructed(position))
                *movelist++ = new_castle(SIDE_TO_MOVE, CASTLE_KING_SIDE);
            if ((castling_rights & CASTLE_QUEEN_SIDE) != CASTLE_NONE && white_queen_side_unobstructed(position))
                *movelist++ = new_castle(SIDE_TO_MOVE, CASTLE_QUEEN_SIDE);
        }
    } else {
        // At this point, there exists exactly one checker whose square index we can get by getting the least
        // significant bit. Notice how we can make this an assignment instead of a bitwise-and. If there were friendly
        // pieces on the line from the king to the attacker, the king would either not be in check, a contradiction, or
        // the attacker would be a knight. But if the attacker is a knight, there is no line between the king and the
        // knight, so only the knight square would be the target.
        target = between_bitboard(king_source, (enum Square)lsb64(checkers));
    }


    /* Pawn moves. */
    const Bitboard friendly_pawns      = piece_occupancy(position, SIDE_TO_MOVE, PIECE_TYPE_PAWN);
    const Bitboard non_promotion_pawns = friendly_pawns & ~RANK_7_BITBOARD;
    Bitboard empty_squares             = ~position->total_occupancy;  // Needed for pawn pushes.

    /* Pawn pushes. */
    // We cleverly use the push_once bitboard to compute the push_twice bitboard as well, masking with target afterwards
    // to make sure we only get the moves we want.
    Bitboard push_once = shift_bitboard_north(non_promotion_pawns) & empty_squares;
    empty_squares &= target;  // We update empty_squares here because we will use it for promotions as well.
    const Bitboard push_twice = shift_bitboard_north(push_once & RANK_3_BITBOARD) & empty_squares;
    push_once &= target;
    movelist = splat_pawn_moves(movelist, push_once, DIRECTION_NORTH);
    movelist = splat_pawn_moves(movelist, push_twice, DIRECTION_NORTH2);

    /* Non-promotion captures. */
    const Bitboard enemies = piece_occupancy_by_color(position, OPPONENT) & target;
    Bitboard attacks_east  = shift_bitboard_northeast(non_promotion_pawns) & enemies;
    Bitboard attacks_west  = shift_bitboard_northwest(non_promotion_pawns) & enemies;
    movelist               = splat_pawn_moves(movelist, attacks_east, DIRECTION_NORTHEAST);
    movelist               = splat_pawn_moves(movelist, attacks_west, DIRECTION_NORTHWEST);

    /* En passant. */
    const enum Square en_passant = en_passant_square(position);
    if (en_passant != SQUARE_NONE) {
        assert(rank_of_square(en_passant) == RANK_6);
        // Notice how we are always considering en passant captures, even if we are in check and the en passant pawn is
        // not the attacker of the king. We deal with this later in is_legal_en_passant().

        // A white pawn attacks the en passant square if a black pawn on the en passant square would be attacking that
        // white pawn.
        Bitboard en_passant_attackers = non_promotion_pawns & piece_base_attacks(PIECE_TYPE_BLACK_PAWN, en_passant);

        while (en_passant_attackers != EMPTY_BITBOARD)
            *movelist++ = new_move((enum Square)pop_lsb64(&en_passant_attackers), en_passant, MOVE_TYPE_EN_PASSANT);
    }

    /* Promotions. */
    const Bitboard promotion_pawns = friendly_pawns & RANK_7_BITBOARD;

    if (promotion_pawns != EMPTY_BITBOARD) {
        push_once = shift_bitboard_north(promotion_pawns)
                  & empty_squares;  // empty_squares was already masked with target.
        attacks_east = shift_bitboard_northeast(promotion_pawns) & enemies;
        attacks_west = shift_bitboard_northwest(promotion_pawns) & enemies;

        enum Square destination;
        while (push_once != EMPTY_BITBOARD) {
            destination = (enum Square)pop_lsb64(&push_once);
            movelist    = new_promotions(movelist, square_south(destination), destination);
        }
        while (attacks_east != EMPTY_BITBOARD) {
            destination = (enum Square)pop_lsb64(&attacks_east);
            movelist    = new_promotions(movelist, square_southwest(destination), destination);
        }
        while (attacks_west != EMPTY_BITBOARD) {
            destination = (enum Square)pop_lsb64(&attacks_west);
            movelist    = new_promotions(movelist, square_southeast(destination), destination);
        }
    }


    /* Knight moves. */
    Bitboard knights = piece_occupancy(position, SIDE_TO_MOVE, PIECE_TYPE_KNIGHT);
    while (knights != EMPTY_BITBOARD) {
        enum Square knight_square = (enum Square)pop_lsb64(&knights);
        movelist = splat_piece_moves(movelist, piece_base_attacks(PIECE_TYPE_KNIGHT, knight_square) & target,
                                     knight_square);
    }


    /* Bishop/Queen moves. */
    Bitboard bishops_and_queens = bishop_queen_occupancy(position, SIDE_TO_MOVE);
    while (bishops_and_queens != EMPTY_BITBOARD) {
        enum Square piece_square = (enum Square)pop_lsb64(&bishops_and_queens);
        movelist = splat_piece_moves(movelist, bishop_attacks(piece_square, position->total_occupancy) & target,
                                     piece_square);
    }


    /* Rook/Queen moves. */
    Bitboard rooks_and_queens = rook_queen_occupancy(position, SIDE_TO_MOVE);
    while (rooks_and_queens != EMPTY_BITBOARD) {
        enum Square piece_square = (enum Square)pop_lsb64(&rooks_and_queens);
        movelist = splat_piece_moves(movelist, rook_attacks(piece_square, position->total_occupancy) & target,
                                     piece_square);
    }

    return movelist;
}

// Computes all pseudolegal moves for black in `position`, stores them in `movelist` and returns the end of the array. A
// pseudolegal move here, is defined as a normal move that may or may not put the side to move in check. So in order to
// filter out the legal moves, we would have to check that a non-king piece does not reveal an attack on the king or
// that the king destination square is not attacked.
static Move* black_pseudolegal_moves(const struct Position* position, Move movelist[static MAX_MOVES]) {
    assert(position != nullptr);
    assert(movelist != nullptr);

    constexpr enum Color SIDE_TO_MOVE = COLOR_BLACK;
    constexpr enum Color OPPONENT     = COLOR_WHITE;

    // All squares that do not contain one of our own pieces are potential targets.
    Bitboard target = ~position->occupancy_by_color[SIDE_TO_MOVE];


    /* Regular king moves. */
    const enum Square king_source = king_square(position, SIDE_TO_MOVE);
    movelist = splat_piece_moves(movelist, piece_base_attacks(PIECE_TYPE_KING, king_source) & target, king_source);

    const Bitboard checkers = position->info->checkers;

    // If we are in double check, only non-castling king moves can get us out of check.
    if (popcount64_greater_than_one(checkers))
        return movelist;  // Immediately return as there are no other possible moves.

    // If not in check, castling moves should be generated. Else, we update the target such that each move will either
    // interpose the check or capture the checker (we have already computed king moves).
    if (checkers == EMPTY_BITBOARD) {
        /* Castling moves. */
        const enum CastlingRights castling_rights = position->info->castling_rights & CASTLE_BLACK;
        if (castling_rights != CASTLE_NONE) {
            if ((castling_rights & CASTLE_KING_SIDE) != CASTLE_NONE && black_king_side_unobstructed(position))
                *movelist++ = new_castle(SIDE_TO_MOVE, CASTLE_KING_SIDE);
            if ((castling_rights & CASTLE_QUEEN_SIDE) != CASTLE_NONE && black_queen_side_unobstructed(position))
                *movelist++ = new_castle(SIDE_TO_MOVE, CASTLE_QUEEN_SIDE);
        }
    } else {
        // At this point, there exists exactly one checker whose square index we can get by getting the least
        // significant bit. Notice how we can make this an assignment instead of a bitwise-and. If there were friendly
        // pieces on the line from the king to the attacker, the king would either not be in check, a contradiction, or
        // the attacker would be a knight. But if the attacker is a knight, there is no line between the king and the
        // knight, so only the knight square would be the target.
        target = between_bitboard(king_source, (enum Square)lsb64(checkers));
    }


    /* Pawn moves. */
    const Bitboard friendly_pawns      = piece_occupancy(position, SIDE_TO_MOVE, PIECE_TYPE_PAWN);
    const Bitboard non_promotion_pawns = friendly_pawns & ~RANK_2_BITBOARD;
    Bitboard empty_squares             = ~position->total_occupancy;  // Needed for pawn pushes.

    /* Pawn pushes. */
    // We cleverly use the push_once bitboard to compute the push_twice bitboard as well, masking with target afterwards
    // to make sure we only get the moves we want.
    Bitboard push_once = shift_bitboard_south(non_promotion_pawns) & empty_squares;
    empty_squares &= target;  // We update empty_squares here because we will use it for promotions as well.
    const Bitboard push_twice = shift_bitboard_south(push_once & RANK_6_BITBOARD) & empty_squares;
    push_once &= target;
    movelist = splat_pawn_moves(movelist, push_once, DIRECTION_SOUTH);
    movelist = splat_pawn_moves(movelist, push_twice, DIRECTION_SOUTH2);

    /* Non-promotion captures. */
    const Bitboard enemies = piece_occupancy_by_color(position, OPPONENT) & target;
    Bitboard attacks_east  = shift_bitboard_southeast(non_promotion_pawns) & enemies;
    Bitboard attacks_west  = shift_bitboard_southwest(non_promotion_pawns) & enemies;
    movelist               = splat_pawn_moves(movelist, attacks_east, DIRECTION_SOUTHEAST);
    movelist               = splat_pawn_moves(movelist, attacks_west, DIRECTION_SOUTHWEST);

    /* En passant. */
    const enum Square en_passant = en_passant_square(position);
    if (en_passant != SQUARE_NONE) {
        assert(rank_of_square(en_passant) == RANK_3);
        // Notice how we are always considering en passant captures, even if we are in check and the en passant pawn is
        // not the attacker of the king. We deal with this later in is_legal_en_passant().

        // A black pawn attacks the en passant square if a white pawn on the en passant square would be attacking that
        // black pawn.
        Bitboard en_passant_attackers = non_promotion_pawns & piece_base_attacks(PIECE_TYPE_WHITE_PAWN, en_passant);

        while (en_passant_attackers != EMPTY_BITBOARD)
            *movelist++ = new_move((enum Square)pop_lsb64(&en_passant_attackers), en_passant, MOVE_TYPE_EN_PASSANT);
    }

    /* Promotions. */
    const Bitboard promotion_pawns = friendly_pawns & RANK_2_BITBOARD;

    if (promotion_pawns != EMPTY_BITBOARD) {
        push_once = shift_bitboard_south(promotion_pawns)
                  & empty_squares;  // empty_squares was already masked with target.
        attacks_east = shift_bitboard_southeast(promotion_pawns) & enemies;
        attacks_west = shift_bitboard_southwest(promotion_pawns) & enemies;

        enum Square destination;
        while (push_once != EMPTY_BITBOARD) {
            destination = (enum Square)pop_lsb64(&push_once);
            movelist    = new_promotions(movelist, square_north(destination), destination);
        }
        while (attacks_east != EMPTY_BITBOARD) {
            destination = (enum Square)pop_lsb64(&attacks_east);
            movelist    = new_promotions(movelist, square_northwest(destination), destination);
        }
        while (attacks_west != EMPTY_BITBOARD) {
            destination = (enum Square)pop_lsb64(&attacks_west);
            movelist    = new_promotions(movelist, square_northeast(destination), destination);
        }
    }


    /* Knight moves. */
    Bitboard knights = piece_occupancy(position, SIDE_TO_MOVE, PIECE_TYPE_KNIGHT);
    while (knights != EMPTY_BITBOARD) {
        enum Square knight_square = (enum Square)pop_lsb64(&knights);
        movelist = splat_piece_moves(movelist, piece_base_attacks(PIECE_TYPE_KNIGHT, knight_square) & target,
                                     knight_square);
    }


    /* Bishop/Queen moves. */
    Bitboard bishops_and_queens = bishop_queen_occupancy(position, SIDE_TO_MOVE);
    while (bishops_and_queens != EMPTY_BITBOARD) {
        enum Square piece_square = (enum Square)pop_lsb64(&bishops_and_queens);
        movelist = splat_piece_moves(movelist, bishop_attacks(piece_square, position->total_occupancy) & target,
                                     piece_square);
    }


    /* Rook/Queen moves. */
    Bitboard rooks_and_queens = rook_queen_occupancy(position, SIDE_TO_MOVE);
    while (rooks_and_queens != EMPTY_BITBOARD) {
        enum Square piece_square = (enum Square)pop_lsb64(&rooks_and_queens);
        movelist = splat_piece_moves(movelist, rook_attacks(piece_square, position->total_occupancy) & target,
                                     piece_square);
    }

    return movelist;
}


// Returns whether a pseudolegal king `move` is legal in `position`.
static INLINE bool is_legal_king_move(const struct Position* position, const Move move) {
    assert(position != nullptr);
    assert(!is_weird_move(move));
    assert(move_source(move) == king_square(position, position->side_to_move));

    // When entering this function, it is assumed that the king is not in check if the move is a castle move. For every
    // king move, we need to at least check that the destination square is not being attacked. Therefore, for a castle
    // move, we have to perform only one extra test: check whether the square in between the source and destination is
    // being attacked.

    // Squares that king traverses while travelling to the index square.
    // clang-format off
    static enum Square traversed_squares[SQUARE_COUNT] = {
        [SQUARE_G1] = SQUARE_F1,
        [SQUARE_C1] = SQUARE_D1,
        [SQUARE_G8] = SQUARE_F8,
        [SQUARE_C8] = SQUARE_D8
    };
    // clang-format on

    const enum Square destination = move_destination(move);
    const enum Color opponent     = opposite_color(position->side_to_move);

    if (type_of_move(move) == MOVE_TYPE_CASTLE
        && square_is_attacked(position, opponent, traversed_squares[destination], position->total_occupancy))
        return false;

    // We bitwise-xor the total occupancy with the bitboard of our king for the case where the destination square lies
    // on the same line as the attacker.
    return !square_is_attacked(position, opponent, destination,
                               position->total_occupancy ^ king_occupancy(position, position->side_to_move));
}

// Returns whether a pseudolegal pinned piece `move` is legal.
static INLINE bool is_legal_pinned_move(const struct Position* position, const Move move) {
    assert(position != nullptr);
    assert(!is_weird_move(move));
    assert(type_of_move(move) != MOVE_TYPE_CASTLE);

    // A pinned piece is only allowed to move on the line of the pinning piece. This can only be the case if this line
    // crosses the king.
    return (line_bitboard(move_source(move), move_destination(move)) & king_occupancy(position, position->side_to_move))
        != EMPTY_BITBOARD;
}

// Returns whether a pseudolegal en passant capture `move` is legal.
static bool is_legal_en_passant(const struct Position* position, const Move move) {
    assert(position != nullptr);
    assert(!is_weird_move(move));
    assert(type_of_move(move) == MOVE_TYPE_EN_PASSANT);

    // At this point, we have already checked that the pawn is not directly pinned. There is still an edge case where
    // the capture can put the king in check. It is also possible that the en passant pawn is giving check currently. To
    // make sure neither of these are the case, we essentially perform the move and make sure it is not check. These
    // edge cases will be rare anyways, so the performance impact is negligible.
    const Bitboard source_bitboard      = square_bitboard(move_source(move));
    const Bitboard destination_bitboard = square_bitboard(move_destination(move));
    const enum Color side_to_move       = position->side_to_move;
    const Bitboard captured_bitboard    = (side_to_move == COLOR_WHITE) ? shift_bitboard_south(destination_bitboard)
                                                                        : shift_bitboard_north(destination_bitboard);

    const enum Square king    = king_square(position, side_to_move);
    const enum Color opponent = opposite_color(side_to_move);

    // The updated occupancy needs the friendly pawn removed from the source and the enemy pawn removed since they may
    // be indirectly pinned by a rook/queen. The friendly pawn needs to be placed on the destination square since it
    // might block a check from a rook/queen that x-rays the en passant pawn to the king.
    const Bitboard occupancy = (position->total_occupancy | destination_bitboard) ^ source_bitboard ^ captured_bitboard;

    // We manually inline square_is_attacked() here and modify it slightly (sorry for the mess). It is guaranteed that
    // the king is not attacked by the opponent king. Besides that, we remove the en passant pawn from the enemy
    // attackers.
    const bool king_is_attacked = ((piece_base_attacks(PIECE_TYPE_BISHOP, king)
                                    & bishop_queen_occupancy(position, opponent))
                                   != EMPTY_BITBOARD
                                   && (bishop_attacks(king, occupancy) & bishop_queen_occupancy(position, opponent))
                                      != EMPTY_BITBOARD)
                               || ((piece_base_attacks(PIECE_TYPE_ROOK, king)
                                    & rook_queen_occupancy(position, opponent))
                                   != EMPTY_BITBOARD
                                   && (rook_attacks(king, occupancy) & rook_queen_occupancy(position, opponent))
                                      != EMPTY_BITBOARD)
                               || ((piece_base_attacks(PIECE_TYPE_KNIGHT, king)
                                    & piece_occupancy(position, opponent, PIECE_TYPE_KNIGHT))
                                   != EMPTY_BITBOARD)
                               || ((piece_base_attacks(pawn_type_from_color(side_to_move), king)
                                    & (piece_occupancy(position, opponent, PIECE_TYPE_PAWN) ^ captured_bitboard))
                                   != EMPTY_BITBOARD);

    return !king_is_attacked;
}


size_t generate_legal_moves(const struct Position* position, Move movelist[static MAX_MOVES]) {
    assert(position != nullptr);
    assert(movelist != nullptr);

    const enum Color side_to_move = position->side_to_move;

    Move* current = movelist;
    movelist      = (side_to_move == COLOR_WHITE) ? white_pseudolegal_moves(position, movelist)
                                                  : black_pseudolegal_moves(position, movelist);

    const Bitboard pinned  = position->info->blockers[side_to_move] & piece_occupancy_by_color(position, side_to_move);
    const enum Square king = king_square(position, side_to_move);

    size_t size = 0;
    while (current != movelist) {
        const enum Square source = move_source(*current);

        // To make sure a pseudolegal move is legal, we need to check whether it puts our king in check, which is only
        // possible if we move the king, if we move a pinned piece or if we capture en passant.
        if ((source == king && !is_legal_king_move(position, *current))
            || ((pinned & square_bitboard(source)) != EMPTY_BITBOARD && !is_legal_pinned_move(position, *current))
            || (type_of_move(*current) == MOVE_TYPE_EN_PASSANT && !is_legal_en_passant(position, *current))) {
            *current = *(--movelist);
        } else {
            ++current;
            ++size;
        }
    }

    return size;
}
