#include "move_generation.h"

#include <assert.h>
#include <stdlib.h>

#include "bitboard.h"
#include "position.h"
#include "types.h"
#include "util.h"



static inline Move* splat_pawn_moves(Move* movelist, Bitboard pawn_moves, enum Direction step) {
    assert(movelist != nullptr);

    while (pawn_moves != EMPTY_BITBOARD) {
        enum Square destination = (enum Square)pop_lsb64(&pawn_moves);
        *movelist++             = new_move(destination - (enum Square)step, destination, MOVE_TYPE_NORMAL);
    }

    return movelist;
}

static inline Move* splat_piece_moves(Move* movelist, Bitboard moves, enum Square source) {
    assert(movelist != nullptr);
    assert(is_valid_square(source));

    while (moves != EMPTY_BITBOARD)
        *movelist++ = new_move(source, (enum Square)pop_lsb64(&moves), MOVE_TYPE_NORMAL);

    return movelist;
}


static Move* white_pawn_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != nullptr);
    assert(movelist != nullptr);

    const Bitboard non_promotion_pawns = piece_occupancy(position, COLOR_WHITE, PIECE_TYPE_PAWN)
                                       & ~RANK_7_BITBOARD;
    const Bitboard enemies = piece_occupancy_by_color(position, COLOR_BLACK) & target;
    Bitboard empty_squares = ~position->total_occupancy;  // For pawn pushes.

    /* Pawn pushes. */
    Bitboard push_once = shift_bitboard_north(non_promotion_pawns) & empty_squares;
    empty_squares &= target;
    Bitboard push_twice = shift_bitboard_north(push_once & RANK_3_BITBOARD) & empty_squares;
    push_once &= target;
    movelist = splat_pawn_moves(movelist, push_once, DIRECTION_NORTH);
    movelist = splat_pawn_moves(movelist, push_twice, 2 * DIRECTION_NORTH);

    /* Non-promotion captures. */
    Bitboard attacks_east = shift_bitboard_northeast(non_promotion_pawns) & enemies;
    Bitboard attacks_west = shift_bitboard_northwest(non_promotion_pawns) & enemies;
    movelist              = splat_pawn_moves(movelist, attacks_east, DIRECTION_NORTHEAST);
    movelist              = splat_pawn_moves(movelist, attacks_west, DIRECTION_NORTHWEST);

    if (position->en_passant_square != SQUARE_NONE) {
        assert(rank_of_square(position->en_passant_square) == RANK_6);

        Bitboard en_passant_attackers = non_promotion_pawns
                                      & piece_base_attacks(PIECE_TYPE_BLACK_PAWN, position->en_passant_square);

        while (en_passant_attackers != EMPTY_BITBOARD)
            *movelist++ = new_move((enum Square)pop_lsb64(&en_passant_attackers), position->en_passant_square,
                                   MOVE_TYPE_EN_PASSANT);
    }

    /* Promotions. */
    const Bitboard promotion_pawns = piece_occupancy(position, COLOR_WHITE, PIECE_TYPE_PAWN) & rank_bitboard(RANK_7);

    if (promotion_pawns != EMPTY_BITBOARD) {
        push_once    = shift_bitboard_north(promotion_pawns) & empty_squares;
        attacks_east = shift_bitboard_northeast(promotion_pawns) & enemies;
        attacks_west = shift_bitboard_northwest(promotion_pawns) & enemies;

        enum Square to;
        while (attacks_east != EMPTY_BITBOARD) {
            to       = (enum Square)pop_lsb64(&attacks_east);
            movelist = new_promotions(movelist, to - (enum Square)DIRECTION_NORTHEAST, to);
        }

        while (attacks_west != EMPTY_BITBOARD) {
            to       = (enum Square)pop_lsb64(&attacks_west);
            movelist = new_promotions(movelist, to - (enum Square)DIRECTION_NORTHWEST, to);
        }

        while (push_once != EMPTY_BITBOARD) {
            to       = (enum Square)pop_lsb64(&push_once);
            movelist = new_promotions(movelist, to - (enum Square)DIRECTION_NORTH, to);
        }
    }

    return movelist;
}

static Move* black_pawn_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    const Bitboard non_promotion_pawns = piece_occupancy(position, COLOR_BLACK, PIECE_TYPE_PAWN)
                                       & ~rank_bitboard(RANK_2);
    const Bitboard enemies = position->occupancy_by_color[COLOR_WHITE] & target;
    Bitboard empty_squares = ~position->total_occupancy;  // For pawn pushes.

    /* Pawn pushes. */
    Bitboard push_once = shift_bitboard_south(non_promotion_pawns) & empty_squares;
    empty_squares &= target;
    Bitboard push_twice = shift_bitboard_south(push_once & rank_bitboard(RANK_6)) & empty_squares;
    push_once &= target;
    movelist = splat_pawn_moves(movelist, push_once, DIRECTION_SOUTH);
    movelist = splat_pawn_moves(movelist, push_twice, 2 * DIRECTION_SOUTH);

    /* Non-promotion captures. */
    Bitboard attacks_east = shift_bitboard_southeast(non_promotion_pawns) & enemies;
    Bitboard attacks_west = shift_bitboard_southwest(non_promotion_pawns) & enemies;
    movelist              = splat_pawn_moves(movelist, attacks_east, DIRECTION_SOUTHEAST);
    movelist              = splat_pawn_moves(movelist, attacks_west, DIRECTION_SOUTHWEST);

    if (position->en_passant_square != SQUARE_NONE) {
        assert(rank_of_square(position->en_passant_square) == RANK_3);

        Bitboard en_passant_attackers = non_promotion_pawns
                                      & piece_base_attacks(PIECE_TYPE_WHITE_PAWN, position->en_passant_square);

        while (en_passant_attackers != EMPTY_BITBOARD)
            *movelist++ = new_move((enum Square)pop_lsb64(&en_passant_attackers), position->en_passant_square,
                                   MOVE_TYPE_EN_PASSANT);
    }

    /* Promotions. */
    const Bitboard promotion_pawns = piece_occupancy(position, COLOR_BLACK, PIECE_TYPE_PAWN) & rank_bitboard(RANK_2);

    if (promotion_pawns != EMPTY_BITBOARD) {
        push_once    = shift_bitboard_south(promotion_pawns) & empty_squares;
        attacks_east = shift_bitboard_southeast(promotion_pawns) & enemies;
        attacks_west = shift_bitboard_southwest(promotion_pawns) & enemies;

        enum Square to;
        while (push_once != EMPTY_BITBOARD) {
            to       = (enum Square)pop_lsb64(&push_once);
            movelist = new_promotions(movelist, to - (enum Square)DIRECTION_SOUTH, to);
        }

        while (attacks_east != EMPTY_BITBOARD) {
            to       = (enum Square)pop_lsb64(&attacks_east);
            movelist = new_promotions(movelist, to - (enum Square)DIRECTION_SOUTHEAST, to);
        }

        while (attacks_west != EMPTY_BITBOARD) {
            to       = (enum Square)pop_lsb64(&attacks_west);
            movelist = new_promotions(movelist, to - (enum Square)DIRECTION_SOUTHWEST, to);
        }
    }

    return movelist;
}

static Move* white_knight_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard white_knights = piece_occupancy(position, COLOR_WHITE, PIECE_TYPE_KNIGHT);

    while (white_knights != EMPTY_BITBOARD) {
        enum Square knight_square = (enum Square)pop_lsb64(&white_knights);
        movelist = splat_piece_moves(movelist, piece_base_attacks(PIECE_TYPE_KNIGHT, knight_square) & target,
                                     knight_square);
    }

    return movelist;
}

static Move* black_knight_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard black_knights = piece_occupancy(position, COLOR_BLACK, PIECE_TYPE_KNIGHT);

    while (black_knights != EMPTY_BITBOARD) {
        enum Square knight_square = (enum Square)pop_lsb64(&black_knights);
        movelist = splat_piece_moves(movelist, piece_base_attacks(PIECE_TYPE_KNIGHT, knight_square) & target,
                                     knight_square);
    }

    return movelist;
}

static Move* white_bishop_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard white_bishops = piece_occupancy(position, COLOR_WHITE, PIECE_TYPE_BISHOP);

    while (white_bishops != EMPTY_BITBOARD) {
        enum Square bishop_square = (enum Square)pop_lsb64(&white_bishops);
        movelist = splat_piece_moves(movelist, bishop_attacks(bishop_square, position->total_occupancy) & target,
                                     bishop_square);
    }

    return movelist;
}

static Move* black_bishop_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard black_bishops = piece_occupancy(position, COLOR_BLACK, PIECE_TYPE_BISHOP);

    while (black_bishops != EMPTY_BITBOARD) {
        enum Square bishop_square = (enum Square)pop_lsb64(&black_bishops);
        movelist = splat_piece_moves(movelist, bishop_attacks(bishop_square, position->total_occupancy) & target,
                                     bishop_square);
    }

    return movelist;
}

static Move* white_rook_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard white_rooks = piece_occupancy(position, COLOR_WHITE, PIECE_TYPE_ROOK);

    while (white_rooks != EMPTY_BITBOARD) {
        enum Square rook_square = (enum Square)pop_lsb64(&white_rooks);
        movelist           = splat_piece_moves(movelist, rook_attacks(rook_square, position->total_occupancy) & target,
                                               rook_square);
    }

    return movelist;
}

static Move* black_rook_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard black_rooks = piece_occupancy(position, COLOR_BLACK, PIECE_TYPE_ROOK);

    while (black_rooks != EMPTY_BITBOARD) {
        enum Square rook_square = (enum Square)pop_lsb64(&black_rooks);
        movelist           = splat_piece_moves(movelist, rook_attacks(rook_square, position->total_occupancy) & target,
                                               rook_square);
    }

    return movelist;
}

static Move* white_queen_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard white_queens = piece_occupancy(position, COLOR_WHITE, PIECE_TYPE_QUEEN);

    while (white_queens != EMPTY_BITBOARD) {
        enum Square queen_square = (enum Square)pop_lsb64(&white_queens);
        movelist            = splat_piece_moves(movelist,
                                                (bishop_attacks(queen_square, position->total_occupancy)
                                      | rook_attacks(queen_square, position->total_occupancy))
                                                & target,
                                                queen_square);
    }

    return movelist;
}

static Move* black_queen_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard black_queens = piece_occupancy(position, COLOR_BLACK, PIECE_TYPE_QUEEN);

    while (black_queens != EMPTY_BITBOARD) {
        enum Square queen_square = (enum Square)pop_lsb64(&black_queens);
        movelist            = splat_piece_moves(movelist,
                                                (bishop_attacks(queen_square, position->total_occupancy)
                                      | rook_attacks(queen_square, position->total_occupancy))
                                                & target,
                                                queen_square);
    }

    return movelist;
}

static Move* white_king_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    const enum Square king = king_square(position, COLOR_WHITE);

    movelist = splat_piece_moves(movelist, piece_base_attacks(PIECE_TYPE_KING, king) & target, king);

    /* For pseudo-legal castling moves, we check whether there is any piece in the way or if the king is in check. */
    if (position->castling_rights != CASTLE_NONE && position->checkers[COLOR_WHITE] == EMPTY_BITBOARD) {
        if ((between_bitboard(SQUARE_E1, SQUARE_G1) & position->total_occupancy) == EMPTY_BITBOARD
            && (CASTLE_WHITE_00 & position->castling_rights) != 0)
            *movelist++ = new_castle(CASTLE_WHITE_00);
        if ((between_bitboard(SQUARE_E1, SQUARE_B1) & position->total_occupancy) == EMPTY_BITBOARD
            && (CASTLE_WHITE_000 & position->castling_rights) != 0)
            *movelist++ = new_castle(CASTLE_WHITE_000);
    }

    return movelist;
}

static Move* black_king_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    const enum Square king = king_square(position, COLOR_BLACK);

    movelist = splat_piece_moves(movelist, piece_base_attacks(PIECE_TYPE_KING, king) & target, king);

    /* For pseudo-legal castling moves, we check whether there is any piece in the way or if the king is in check. */
    if (position->castling_rights != CASTLE_NONE && position->checkers[COLOR_BLACK] == EMPTY_BITBOARD) {
        if ((between_bitboard(SQUARE_E8, SQUARE_G8) & position->total_occupancy) == EMPTY_BITBOARD
            && (CASTLE_BLACK_00 & position->castling_rights) != 0)
            *movelist++ = new_castle(CASTLE_BLACK_00);
        if ((between_bitboard(SQUARE_E8, SQUARE_B8) & position->total_occupancy) == EMPTY_BITBOARD
            && (CASTLE_BLACK_000 & position->castling_rights) != 0)
            *movelist++ = new_castle(CASTLE_BLACK_000);
    }

    return movelist;
}

static Move* generate_white_pseudo_legal_moves(const struct Position* position, Move* movelist) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard target = ~position->occupancy_by_color[COLOR_WHITE];

    assert(popcount64(position->checkers[COLOR_WHITE]) <= 2);

    const Bitboard checkers = position->checkers[COLOR_WHITE];

    if (!popcount64_greater_than_one(checkers)) {
        if (checkers != EMPTY_BITBOARD)
            target &= between_bitboard(king_square(position, COLOR_WHITE), (enum Square)lsb64(checkers));

        movelist = white_pawn_pseudo_legal_moves(position, movelist, target);
        movelist = white_knight_pseudo_legal_moves(position, movelist, target);
        movelist = white_bishop_pseudo_legal_moves(position, movelist, target);
        movelist = white_rook_pseudo_legal_moves(position, movelist, target);
        movelist = white_queen_pseudo_legal_moves(position, movelist, target);
    }

    movelist = white_king_pseudo_legal_moves(position, movelist, ~position->occupancy_by_color[COLOR_WHITE]);

    return movelist;
}

Move* generate_black_pseudo_legal_moves(const struct Position* position, Move* movelist) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard target = ~position->occupancy_by_color[COLOR_BLACK];

    assert(popcount64(position->checkers[COLOR_BLACK]) <= 2);

    const Bitboard checkers = position->checkers[COLOR_BLACK];

    if (!popcount64_greater_than_one(checkers)) {
        if (checkers != EMPTY_BITBOARD)
            target &= between_bitboard(king_square(position, COLOR_BLACK), (enum Square)lsb64(checkers));

        movelist = black_pawn_pseudo_legal_moves(position, movelist, target);
        movelist = black_knight_pseudo_legal_moves(position, movelist, target);
        movelist = black_bishop_pseudo_legal_moves(position, movelist, target);
        movelist = black_rook_pseudo_legal_moves(position, movelist, target);
        movelist = black_queen_pseudo_legal_moves(position, movelist, target);
    }

    movelist = black_king_pseudo_legal_moves(position, movelist, ~position->occupancy_by_color[COLOR_BLACK]);

    return movelist;
}

static bool is_legal_king_move(const struct Position* position, Move move);
static inline bool is_legal_pinned_move(const struct Position* position, Move move);
static bool is_legal_en_passant(struct Position* position, Move move);

size_t generate_legal_moves(struct Position* position, Move movelist[static MAX_MOVES]) {
    assert(position != NULL);
    assert(movelist != NULL);

    Move* current = movelist;
    movelist      = (position->side_to_move == COLOR_WHITE) ? generate_white_pseudo_legal_moves(position, movelist)
                                                            : generate_black_pseudo_legal_moves(position, movelist);

    const Bitboard pinned = position->blockers[position->side_to_move]
                          & position->occupancy_by_color[position->side_to_move];
    const enum Square king = king_square(position, position->side_to_move);

    size_t size = 0;
    while (current != movelist) {
        enum Square source = move_source(*current);

        if ((source == king && !is_legal_king_move(position, *current))
            || ((pinned & square_bitboard(source)) != EMPTY_BITBOARD && !is_legal_pinned_move(position, *current))
            || (move_type(*current) == MOVE_TYPE_EN_PASSANT && !is_legal_en_passant(position, *current))) {
            *current = *(--movelist);
        } else {
            ++current;
            ++size;
        }
    }

    return size;
}

static bool is_legal_king_move(const struct Position* position, Move move) {
    assert(position != NULL);
    assert(is_valid_move(move));
    assert(move_source(move) == king_square(position, position->side_to_move));

    // Squares that king traverses while travelling to the index square.
    // clang-format off
    static const Bitboard castling_squares[SQUARE_COUNT] = {
        [SQUARE_G1] = SQUARE_BITBOARD(SQUARE_F1) | SQUARE_BITBOARD(SQUARE_G1),
        [SQUARE_C1] = SQUARE_BITBOARD(SQUARE_D1) | SQUARE_BITBOARD(SQUARE_C1),
        [SQUARE_G8] = SQUARE_BITBOARD(SQUARE_F8) | SQUARE_BITBOARD(SQUARE_G8),
        [SQUARE_C8] = SQUARE_BITBOARD(SQUARE_D8) | SQUARE_BITBOARD(SQUARE_C8)
    };
    // clang-format on

    if (move_type(move) == MOVE_TYPE_CASTLE) {
        /* At this point, for castling moves, we have only checked whether there are pieces in the way and whether the
         * king is in check. We will now check if the king moves over an attacked square. */
        Bitboard squares_to_traverse = castling_squares[move_destination(move)];

        while (squares_to_traverse != EMPTY_BITBOARD) {
            enum Square square = (enum Square)pop_lsb64(&squares_to_traverse);

            if (square_is_attacked(position, !position->side_to_move, square, position->total_occupancy))
                return false;
        }

        return true;
    }

    return square_is_attacked(
           position, !position->side_to_move, move_destination(move),
           position->total_occupancy ^ piece_occupancy(position, position->side_to_move, PIECE_TYPE_KING))
        == EMPTY_BITBOARD;
}

static inline bool is_legal_pinned_move(const struct Position* position, Move move) {
    assert(position != NULL);
    assert(is_valid_move(move));
    assert(move_type(move) == MOVE_TYPE_NORMAL || move_type(move) == MOVE_TYPE_EN_PASSANT
           || move_type(move) == MOVE_TYPE_PROMOTION);

    return (line_bitboard(move_source(move), move_destination(move))
            & piece_occupancy(position, position->side_to_move, PIECE_TYPE_KING))
        != EMPTY_BITBOARD;
}

static bool is_legal_en_passant(struct Position* position, Move move) {
    assert(position != NULL);
    assert(is_valid_move(move));
    assert(move_type(move) == MOVE_TYPE_EN_PASSANT);

    /* At this point, we have already checked that the pawn is not directly pinned. We sort of perform the move and
     * check that our king does not end up in check. */
    const Bitboard source_bitboard      = square_bitboard(move_source(move));
    const Bitboard destination_bitboard = square_bitboard(move_destination(move));
    const Bitboard captured_bitboard    = (position->side_to_move == COLOR_WHITE)
                                        ? shift_bitboard_south(destination_bitboard)
                                        : shift_bitboard_north(destination_bitboard);

    position->occupancy_by_color[!position->side_to_move] ^= captured_bitboard;
    const Bitboard attackers = attackers_of_square(
                               position, king_square(position, position->side_to_move),
                               (position->total_occupancy | destination_bitboard) ^ source_bitboard ^ captured_bitboard)
                             & piece_occupancy_by_color(position, !position->side_to_move);
    position->occupancy_by_color[!position->side_to_move] |= captured_bitboard;

    return attackers == EMPTY_BITBOARD;
}
