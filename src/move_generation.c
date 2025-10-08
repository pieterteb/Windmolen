#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "move_generation.h"

#include "bitboard.h"
#include "position.h"
#include "types.h"
#include "util.h"



static inline Move* splat_pawn_moves(Move* movelist, Bitboard pawn_moves, Direction step) {
    assert(movelist != NULL);

    while (pawn_moves != EMPTY_BITBOARD) {
        Square to   = (Square)pop_lsb64(&pawn_moves);
        *movelist++ = new_move(to - step, to, MOVE_TYPE_NORMAL);
    }

    return movelist;
}

static inline Move* splat_piece_moves(Move* movelist, Bitboard moves, Square from) {
    assert(movelist != NULL);
    assert(is_valid_square(from));

    while (moves != EMPTY_BITBOARD)
        *movelist++ = new_move(from, (Square)pop_lsb64(&moves), MOVE_TYPE_NORMAL);

    return movelist;
}


static Move* white_pawn_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    const Bitboard non_promotion_pawns = position->occupancy_by_piece[PIECE_WHITE_PAWN] & ~rank_bitboard(RANK_7);
    const Bitboard enemies             = position->occupancy_by_color[COLOR_BLACK] & target;
    Bitboard empty_squares             = ~position->total_occupancy; // For pawn pushes.

    /* Pawn pushes. */
    Bitboard push_once   = shift_bitboard(non_promotion_pawns, DIRECTION_NORTH) & empty_squares;
    empty_squares       &= target;
    Bitboard push_twice  = shift_bitboard(push_once & rank_bitboard(RANK_3), DIRECTION_NORTH) & empty_squares;
    push_once           &= target;
    movelist             = splat_pawn_moves(movelist, push_once, DIRECTION_NORTH);
    movelist             = splat_pawn_moves(movelist, push_twice, 2 * DIRECTION_NORTH);

    /* Non-promotion captures. */
    Bitboard attacks_east = shift_bitboard(non_promotion_pawns, DIRECTION_NORTHEAST) & enemies;
    Bitboard attacks_west = shift_bitboard(non_promotion_pawns, DIRECTION_NORTHWEST) & enemies;
    movelist              = splat_pawn_moves(movelist, attacks_east, DIRECTION_NORTHEAST);
    movelist              = splat_pawn_moves(movelist, attacks_west, DIRECTION_NORTHWEST);

    if (position->en_passant_square != SQUARE_NONE) {
        assert(rank_from_square(position->en_passant_square) == RANK_6);

        Bitboard en_passant_attackers = non_promotion_pawns
                                      & piece_base_attacks(PIECE_TYPE_BLACK_PAWN, position->en_passant_square);

        while (en_passant_attackers != EMPTY_BITBOARD)
            *movelist++ = new_move((Square)pop_lsb64(&en_passant_attackers),
                                   position->en_passant_square,
                                   MOVE_TYPE_EN_PASSANT);
    }

    /* Promotions. */
    const Bitboard promotion_pawns = position->occupancy_by_piece[PIECE_WHITE_PAWN] & rank_bitboard(RANK_7);

    if (promotion_pawns != EMPTY_BITBOARD) {
        push_once    = shift_bitboard(promotion_pawns, DIRECTION_NORTH) & empty_squares;
        attacks_east = shift_bitboard(promotion_pawns, DIRECTION_NORTHEAST) & enemies;
        attacks_west = shift_bitboard(promotion_pawns, DIRECTION_NORTHWEST) & enemies;

        Square to;
        while (attacks_east != EMPTY_BITBOARD) {
            to       = (Square)pop_lsb64(&attacks_east);
            movelist = new_promotions(movelist, to - DIRECTION_NORTHEAST, to);
        }

        while (attacks_west != EMPTY_BITBOARD) {
            to       = (Square)pop_lsb64(&attacks_west);
            movelist = new_promotions(movelist, to - DIRECTION_NORTHWEST, to);
        }

        while (push_once != EMPTY_BITBOARD) {
            to       = (Square)pop_lsb64(&push_once);
            movelist = new_promotions(movelist, to - DIRECTION_NORTH, to);
        }
    }

    return movelist;
}

static Move* black_pawn_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    const Bitboard non_promotion_pawns = position->occupancy_by_piece[PIECE_BLACK_PAWN] & ~rank_bitboard(RANK_2);
    const Bitboard enemies             = position->occupancy_by_color[COLOR_WHITE] & target;
    Bitboard empty_squares             = ~position->total_occupancy; // For pawn pushes.

    /* Pawn pushes. */
    Bitboard push_once   = shift_bitboard(non_promotion_pawns, DIRECTION_SOUTH) & empty_squares;
    empty_squares       &= target;
    Bitboard push_twice  = shift_bitboard(push_once & rank_bitboard(RANK_6), DIRECTION_SOUTH) & empty_squares;
    push_once           &= target;
    movelist             = splat_pawn_moves(movelist, push_once, DIRECTION_SOUTH);
    movelist             = splat_pawn_moves(movelist, push_twice, 2 * DIRECTION_SOUTH);

    /* Non-promotion captures. */
    Bitboard attacks_east = shift_bitboard(non_promotion_pawns, DIRECTION_SOUTHEAST) & enemies;
    Bitboard attacks_west = shift_bitboard(non_promotion_pawns, DIRECTION_SOUTHWEST) & enemies;
    movelist              = splat_pawn_moves(movelist, attacks_east, DIRECTION_SOUTHEAST);
    movelist              = splat_pawn_moves(movelist, attacks_west, DIRECTION_SOUTHWEST);

    if (position->en_passant_square != SQUARE_NONE) {
        assert(rank_from_square(position->en_passant_square) == RANK_3);

        Bitboard en_passant_attackers = non_promotion_pawns
                                      & piece_base_attacks(PIECE_TYPE_WHITE_PAWN, position->en_passant_square);

        while (en_passant_attackers != EMPTY_BITBOARD)
            *movelist++ = new_move((Square)pop_lsb64(&en_passant_attackers),
                                   position->en_passant_square,
                                   MOVE_TYPE_EN_PASSANT);
    }

    /* Promotions. */
    const Bitboard promotion_pawns = position->occupancy_by_piece[PIECE_BLACK_PAWN] & rank_bitboard(RANK_2);

    if (promotion_pawns != EMPTY_BITBOARD) {
        push_once    = shift_bitboard(promotion_pawns, DIRECTION_SOUTH) & empty_squares;
        attacks_east = shift_bitboard(promotion_pawns, DIRECTION_SOUTHEAST) & enemies;
        attacks_west = shift_bitboard(promotion_pawns, DIRECTION_SOUTHWEST) & enemies;

        Square to;
        while (push_once != EMPTY_BITBOARD) {
            to       = (Square)pop_lsb64(&push_once);
            movelist = new_promotions(movelist, to - DIRECTION_SOUTH, to);
        }

        while (attacks_east != EMPTY_BITBOARD) {
            to       = (Square)pop_lsb64(&attacks_east);
            movelist = new_promotions(movelist, to - DIRECTION_SOUTHEAST, to);
        }

        while (attacks_west != EMPTY_BITBOARD) {
            to       = (Square)pop_lsb64(&attacks_west);
            movelist = new_promotions(movelist, to - DIRECTION_SOUTHWEST, to);
        }
    }

    return movelist;
}

static Move* white_knight_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard white_knights = position->occupancy_by_piece[PIECE_WHITE_KNIGHT];

    while (white_knights != EMPTY_BITBOARD) {
        Square knight_square = (Square)pop_lsb64(&white_knights);
        movelist             = splat_piece_moves(movelist,
                                     piece_base_attacks(PIECE_TYPE_KNIGHT, knight_square) & target,
                                     knight_square);
    }

    return movelist;
}

static Move* black_knight_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard black_knights = position->occupancy_by_piece[PIECE_BLACK_KNIGHT];

    while (black_knights != EMPTY_BITBOARD) {
        Square knight_square = (Square)pop_lsb64(&black_knights);
        movelist             = splat_piece_moves(movelist,
                                     piece_base_attacks(PIECE_TYPE_KNIGHT, knight_square) & target,
                                     knight_square);
    }

    return movelist;
}

static Move* white_bishop_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard white_bishops = position->occupancy_by_piece[PIECE_WHITE_BISHOP];

    while (white_bishops != EMPTY_BITBOARD) {
        Square bishop_square = (Square)pop_lsb64(&white_bishops);
        movelist             = splat_piece_moves(movelist,
                                     bishop_attacks(bishop_square, position->total_occupancy) & target,
                                     bishop_square);
    }

    return movelist;
}

static Move* black_bishop_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard black_bishops = position->occupancy_by_piece[PIECE_BLACK_BISHOP];

    while (black_bishops != EMPTY_BITBOARD) {
        Square bishop_square = (Square)pop_lsb64(&black_bishops);
        movelist             = splat_piece_moves(movelist,
                                     bishop_attacks(bishop_square, position->total_occupancy) & target,
                                     bishop_square);
    }

    return movelist;
}

static Move* white_rook_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard white_rooks = position->occupancy_by_piece[PIECE_WHITE_ROOK];

    while (white_rooks != EMPTY_BITBOARD) {
        Square rook_square = (Square)pop_lsb64(&white_rooks);
        movelist           = splat_piece_moves(movelist,
                                     rook_attacks(rook_square, position->total_occupancy) & target,
                                     rook_square);
    }

    return movelist;
}

static Move* black_rook_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard black_rooks = position->occupancy_by_piece[PIECE_BLACK_ROOK];

    while (black_rooks != EMPTY_BITBOARD) {
        Square rook_square = (Square)pop_lsb64(&black_rooks);
        movelist           = splat_piece_moves(movelist,
                                     rook_attacks(rook_square, position->total_occupancy) & target,
                                     rook_square);
    }

    return movelist;
}

static Move* white_queen_pseudo_legal_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL);
    assert(movelist != NULL);

    Bitboard white_queens = position->occupancy_by_piece[PIECE_WHITE_QUEEN];

    while (white_queens != EMPTY_BITBOARD) {
        Square queen_square = (Square)pop_lsb64(&white_queens);
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

    Bitboard black_queens = position->occupancy_by_piece[PIECE_BLACK_QUEEN];

    while (black_queens != EMPTY_BITBOARD) {
        Square queen_square = (Square)pop_lsb64(&black_queens);
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

    const Square king = king_square(position, COLOR_WHITE);

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

    const Square king = king_square(position, COLOR_BLACK);

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
            target &= between_bitboard(king_square(position, COLOR_WHITE), (Square)lsb64(checkers));

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
            target &= between_bitboard(king_square(position, COLOR_BLACK), (Square)lsb64(checkers));

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

    if (position->fullmove_counter == 100) return 0;

    Move* current = movelist;
    movelist      = (position->side_to_move == COLOR_WHITE) ? generate_white_pseudo_legal_moves(position, movelist)
                                                            : generate_black_pseudo_legal_moves(position, movelist);

    const Bitboard pinned = position->blockers[position->side_to_move]
                          & position->occupancy_by_color[position->side_to_move];
    const Square king = king_square(position, position->side_to_move);

    size_t size = 0;
    while (current != movelist) {
        Square source = move_source(*current);

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
            Square square = (Square)pop_lsb64(&squares_to_traverse);

            if (square_is_attacked(position, !position->side_to_move, square, position->total_occupancy)) return false;
        }

        return true;
    }

    return square_is_attacked(position,
                              !position->side_to_move,
                              move_destination(move),
                              position->total_occupancy
                              ^ piece_occupancy(position, position->side_to_move, PIECE_TYPE_KING))
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
                                        ? shift_bitboard(destination_bitboard, DIRECTION_SOUTH)
                                        : shift_bitboard(destination_bitboard, DIRECTION_NORTH);

    position->occupancy_by_color[!position->side_to_move] ^= captured_bitboard;
    const Bitboard attackers                               = attackers_of_square(position,
                                                   king_square(position, position->side_to_move),
                                                   (position->total_occupancy | destination_bitboard) ^ source_bitboard
                                                   ^ captured_bitboard)
                             & piece_occupancy_by_color(position, !position->side_to_move);
    position->occupancy_by_color[!position->side_to_move] |= captured_bitboard;

    return attackers == EMPTY_BITBOARD;
}


void print_move(FILE* stream, Move move) {
    assert(stream != NULL);
    assert(is_valid_move(move));

    // clang-format off
    static const char square_to_string[SQUARE_COUNT][3] = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
    };
    // clang-format on

    fprintf(stream, "%s%s", square_to_string[move_source(move)], square_to_string[move_destination(move)]);

    if (move_type(move) == MOVE_TYPE_PROMOTION) fputc(promotion_to_char(move), stream);
}
