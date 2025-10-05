#include <assert.h>
#include <stdlib.h>

#include "move_generation.h"
#include "bitboard.h"
#include "position.h"
#include "types.h"
#include "util.h"



static inline Move* splat_pawn_moves(Move* movelist, Bitboard pawn_moves, Direction step) {
    assert(movelist != NULL);

    while (pawn_moves != EMPTY_BITBOARD) {
        Square to = (Square)pop_lsb64(&pawn_moves);
        *movelist++ = new_move(to - step, to, MOVE_TYPE_NORMAL);
    }

    return movelist;
}

static inline Move* splat_piece_moves(Move* movelist, Bitboard moves, Square from) {
    assert (movelist != NULL && is_valid_square(from));

    while (moves != EMPTY_BITBOARD)
        *movelist++ = new_move(from, (Square)pop_lsb64(&moves), MOVE_TYPE_NORMAL);

    return movelist;
}


static Move* white_pawn_pseudo_moves(const Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    const Bitboard regular_pawns = position->board[PIECE_WHITE_PAWN] & ~RANK_7_BITBOARD;
    const Bitboard enemies = position->occupancy[COLOR_BLACK] & target;
    const Bitboard empty_squares = ~position->total_occupancy & target; // For pawn pushes.

    /* Pawn pushes. */
    Bitboard push_once = shift_bitboard(regular_pawns, DIRECTION_NORTH) & empty_squares;
    Bitboard push_twice = shift_bitboard(push_once & RANK_3_BITBOARD, DIRECTION_NORTH) & empty_squares;
    movelist = splat_pawn_moves(movelist, push_once, DIRECTION_NORTH);
    movelist = splat_pawn_moves(movelist, push_twice, 2 * DIRECTION_NORTH);

    /* Non-promotion captures. */
    Bitboard attacks_right = shift_bitboard(regular_pawns, DIRECTION_NORTHEAST) & enemies;
    Bitboard attacks_left = shift_bitboard(regular_pawns, DIRECTION_NORTHWEST) & enemies;
    movelist = splat_pawn_moves(movelist, attacks_right, DIRECTION_NORTHEAST);
    movelist = splat_pawn_moves(movelist, attacks_left, DIRECTION_NORTHWEST);

    if (position->en_passant_square != SQUARE_NONE) {
        assert(rank_from_square(position->en_passant_square) == RANK_6);

        Bitboard en_passant_attackers = regular_pawns & piece_base_attack(PIECE_BLACK_PAWN, position->en_passant_square);

        while (en_passant_attackers != EMPTY_BITBOARD)
            *movelist++ = new_move((Square)pop_lsb64(&en_passant_attackers), position->en_passant_square, MOVE_TYPE_EN_PASSANT);
    }

    /* Promotions. */
    const Bitboard promotion_pawns = position->board[PIECE_WHITE_PAWN] & RANK_7_BITBOARD;
    
    if (promotion_pawns != EMPTY_BITBOARD) {
        attacks_right = shift_bitboard(promotion_pawns, DIRECTION_NORTHEAST);
        attacks_left = shift_bitboard(promotion_pawns, DIRECTION_NORTHWEST);
        push_once = shift_bitboard(promotion_pawns, DIRECTION_NORTH) & empty_squares;

        Square to;
        while (attacks_right != EMPTY_BITBOARD) {
            to = (Square)pop_lsb64(&attacks_right);
            movelist = new_promotions(movelist, to - DIRECTION_NORTHEAST, to);
        }

        while (attacks_left != EMPTY_BITBOARD) {
            to = (Square)pop_lsb64(&attacks_left);
            movelist = new_promotions(movelist, to - DIRECTION_NORTHWEST, to);
        }

        while (push_once != EMPTY_BITBOARD) {
            to = (Square)pop_lsb64(&push_once);
            movelist = new_promotions(movelist, to - DIRECTION_NORTH, to);
        }
    }

    return movelist;
}

static Move* black_pawn_pseudo_moves(const Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    const Bitboard regular_pawns = position->board[PIECE_BLACK_PAWN] & ~RANK_2_BITBOARD;
    const Bitboard enemies = position->occupancy[COLOR_WHITE] & target;
    const Bitboard empty_squares = ~position->total_occupancy & target; // For pawn pushes.

    /* Pawn pushes. */
    Bitboard push_once = shift_bitboard(regular_pawns, DIRECTION_SOUTH) & empty_squares;
    Bitboard push_twice = shift_bitboard(push_once & RANK_6_BITBOARD, DIRECTION_SOUTH) & empty_squares;
    movelist = splat_pawn_moves(movelist, push_once, DIRECTION_SOUTH);
    movelist = splat_pawn_moves(movelist, push_twice, 2 * DIRECTION_SOUTH);

    /* Non-promotion captures. */
    Bitboard attacks_right = shift_bitboard(regular_pawns, DIRECTION_SOUTHWEST) & enemies;
    Bitboard attacks_left = shift_bitboard(regular_pawns, DIRECTION_SOUTHEAST) & enemies;
    movelist = splat_pawn_moves(movelist, attacks_right, DIRECTION_SOUTHWEST);
    movelist = splat_pawn_moves(movelist, attacks_left, DIRECTION_SOUTHEAST);

    if (position->en_passant_square != SQUARE_NONE) {
        assert(rank_from_square(position->en_passant_square) == RANK_3);

        Bitboard en_passant_attackers = regular_pawns & piece_base_attack(PIECE_WHITE_PAWN, position->en_passant_square);

        while (en_passant_attackers != EMPTY_BITBOARD)
            *movelist++ = new_move((Square)pop_lsb64(&en_passant_attackers), position->en_passant_square, MOVE_TYPE_EN_PASSANT);
    }

    /* Promotions. */
    const Bitboard promotion_pawns = position->board[PIECE_BLACK_PAWN] & RANK_2_BITBOARD;
    
    if (promotion_pawns != EMPTY_BITBOARD) {
        attacks_right = shift_bitboard(promotion_pawns, DIRECTION_SOUTHWEST);
        attacks_left = shift_bitboard(promotion_pawns, DIRECTION_SOUTHEAST);
        push_once = shift_bitboard(promotion_pawns, DIRECTION_SOUTH) & empty_squares;

        Square to;
        while (attacks_right != EMPTY_BITBOARD) {
            to = (Square)pop_lsb64(&attacks_right);
            movelist = new_promotions(movelist, to - DIRECTION_SOUTHWEST, to);
        }

        while (attacks_left != EMPTY_BITBOARD) {
            to = (Square)pop_lsb64(&attacks_left);
            movelist = new_promotions(movelist, to - DIRECTION_SOUTHEAST, to);
        }

        while (push_once != EMPTY_BITBOARD) {
            to = (Square)pop_lsb64(&push_once);
            movelist = new_promotions(movelist, to - DIRECTION_SOUTH, to);
        }
    }

    return movelist;
}

static Move* pawn_pseudo_moves(const Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    return (position->side_to_move == COLOR_WHITE) ? white_pawn_pseudo_moves(position, movelist, target) : black_pawn_pseudo_moves(position, movelist, target);
}

static Move* white_knight_pseudo_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    Bitboard white_knights = position->board[PIECE_WHITE_KNIGHT];

    while (white_knights != EMPTY_BITBOARD) {
        Square knight_square = (Square)pop_lsb64(&white_knights);
        movelist = splat_piece_moves(movelist, piece_base_attack(PIECE_TYPE_KNIGHT, knight_square) & target, knight_square);
    }

    return movelist;
}

static Move* black_knight_pseudo_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    Bitboard black_knights = position->board[PIECE_BLACK_KNIGHT];

    while (black_knights != EMPTY_BITBOARD) {
        Square knight_square = (Square)pop_lsb64(&black_knights);
        movelist = splat_piece_moves(movelist, piece_base_attack(PIECE_TYPE_KNIGHT, knight_square) & target, knight_square);
    }

    return movelist;
}

static Move* knight_pseudo_moves(const Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    return (position->side_to_move == COLOR_WHITE) ? white_knight_pseudo_moves(position, movelist, target) : black_knight_pseudo_moves(position, movelist, target);
}

static Move* white_bishop_pseudo_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    Bitboard white_bishops = position->board[PIECE_WHITE_BISHOP];
    
    while (white_bishops != EMPTY_BITBOARD) {
        Square bishop_square = (Square)pop_lsb64(&white_bishops);
        movelist = splat_piece_moves(movelist, slider_attacks(PIECE_TYPE_BISHOP, bishop_square, position->total_occupancy) & target, bishop_square);
    }

    return movelist;
}

static Move* black_bishop_pseudo_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    Bitboard black_bishops = position->board[PIECE_BLACK_BISHOP];
    
    while (black_bishops != EMPTY_BITBOARD) {
        Square bishop_square = (Square)pop_lsb64(&black_bishops);
        movelist = splat_piece_moves(movelist, slider_attacks(PIECE_TYPE_BISHOP, bishop_square, position->total_occupancy) & target, bishop_square);
    }

    return movelist;
}

static Move* bishop_pseudo_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    return (position->side_to_move == COLOR_WHITE) ? white_bishop_pseudo_moves(position, movelist, target) : black_bishop_pseudo_moves(position, movelist, target);
}

static Move* white_rook_pseudo_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    Bitboard white_rooks = position->board[PIECE_WHITE_ROOK];

    while (white_rooks != EMPTY_BITBOARD) {
        Square rook_square = (Square)pop_lsb64(&white_rooks);
        movelist = splat_piece_moves(movelist, slider_attacks(PIECE_TYPE_ROOK, rook_square, position->total_occupancy) & target, rook_square);
    }

    return movelist;
}

static Move* black_rook_pseudo_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    Bitboard black_rooks = position->board[PIECE_BLACK_ROOK];

    while (black_rooks != EMPTY_BITBOARD) {
        Square rook_square = (Square)pop_lsb64(&black_rooks);
        movelist = splat_piece_moves(movelist, slider_attacks(PIECE_TYPE_ROOK, rook_square, position->total_occupancy) & target, rook_square);
    }

    return movelist;
}

static Move* rook_pseudo_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    return (position->side_to_move == COLOR_WHITE) ? white_rook_pseudo_moves(position, movelist, target) : black_rook_pseudo_moves(position, movelist, target);
}

static Move* white_queen_pseudo_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    Bitboard white_queens = position->board[PIECE_WHITE_QUEEN];

    while (white_queens != EMPTY_BITBOARD) {
        Square queen_square = (Square)pop_lsb64(&white_queens);
        movelist = splat_piece_moves(movelist, slider_attacks(PIECE_TYPE_BISHOP, queen_square, position->total_occupancy) & target, queen_square);
        movelist = splat_piece_moves(movelist, slider_attacks(PIECE_TYPE_ROOK, queen_square, position->total_occupancy) & target, queen_square);
    }

    return movelist;
}

static Move* black_queen_pseudo_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    Bitboard black_queens = position->board[PIECE_BLACK_QUEEN];

    while (black_queens != EMPTY_BITBOARD) {
        Square queen_square = (Square)pop_lsb64(&black_queens);
        movelist = splat_piece_moves(movelist, slider_attacks(PIECE_TYPE_BISHOP, queen_square, position->total_occupancy) & target, queen_square);
        movelist = splat_piece_moves(movelist, slider_attacks(PIECE_TYPE_ROOK, queen_square, position->total_occupancy) & target, queen_square);
    }

    return movelist;
}

static Move* queen_pseudo_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    return (position->side_to_move == COLOR_WHITE) ? white_queen_pseudo_moves(position, movelist, target) : black_queen_pseudo_moves(position, movelist, target);
}

static Move* white_king_pseudo_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    Square king_square = (Square)lsb64(position->board[PIECE_WHITE_KING]);

    movelist = splat_piece_moves(movelist, piece_base_attack(PIECE_TYPE_KING, king_square) & target, king_square);

    // For pseudo-legal castling moves, we check whether there is any piece in the way or if the king is in check.
    if (position->castling_rights != NO_CASTLING && position->checkers[COLOR_WHITE] == EMPTY_BITBOARD) {
        if ((WHITE_00 & position->castling_rights) && (position->castling_squares[WHITE_00] & position->total_occupancy) == EMPTY_BITBOARD)
            *movelist++ = new_castle(WHITE_00);
        if ((WHITE_000 & position->castling_rights) && (position->castling_squares[WHITE_000] & position->total_occupancy) == EMPTY_BITBOARD)
            *movelist++ = new_castle(WHITE_000);
    }

    return movelist;
}

static Move* black_king_pseudo_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    Square king_square = (Square)lsb64(position->board[PIECE_BLACK_KING]);

    movelist = splat_piece_moves(movelist, piece_base_attack(PIECE_TYPE_KING, king_square) & target, king_square);

    // For pseudo-legal castling moves, we check whether there is any piece in the way or if the king is in check.
    if (position->castling_rights != NO_CASTLING && position->checkers[COLOR_BLACK] == EMPTY_BITBOARD) {
        if ((BLACK_00 & position->castling_rights) && (position->castling_squares[BLACK_00] & position->total_occupancy) == EMPTY_BITBOARD)
            *movelist++ = new_castle(BLACK_00);
        if ((BLACK_000 & position->castling_rights) && (position->castling_squares[BLACK_000] & position->total_occupancy) == EMPTY_BITBOARD)
            *movelist++ = new_castle(BLACK_000);
    }

    return movelist;
}

static Move* king_pseudo_moves(const struct Position* position, Move* movelist, Bitboard target) {
    assert(position != NULL && movelist != NULL);

    return (position->side_to_move == COLOR_WHITE) ? white_king_pseudo_moves(position, movelist, target) : black_king_pseudo_moves(position, movelist, target);
}

static Move* generate_pseudo_legal_moves(const Position* position, Move* movelist) {
    assert(position != NULL && movelist != NULL);

    Bitboard target = ~position->occupancy[position->side_to_move];

    movelist = pawn_pseudo_moves(position, movelist, target);
    movelist = knight_pseudo_moves(position, movelist, target);
    movelist = bishop_pseudo_moves(position, movelist, target);
    movelist = rook_pseudo_moves(position, movelist, target);
    movelist = queen_pseudo_moves(position, movelist, target);
    movelist = king_pseudo_moves(position, movelist, target);

    return movelist;
}

size_t generate_legal_moves(const struct Position* position, Move movelist[static MAX_MOVES]) {
    assert(position != NULL);
    assert(movelist != NULL);

    if (position->fullmove_counter == 100)
        return 0;

    Move* current = movelist;
    movelist = generate_pseudo_legal_moves(position, movelist);

    const Bitboard pinned = position->blockers[position->side_to_move] & position->occupancy[position->side_to_move];

    Square king_square = (position->side_to_move == COLOR_WHITE) ? (Square)lsb64(position->board[PIECE_WHITE_KING]) : (Square)lsb64(position->board[PIECE_BLACK_KING]);

    size_t size = 0;
    while (current != movelist) {
        if (((pinned & square_bitboard(get_move_source(*current))) != EMPTY_BITBOARD && !is_legal_pinned_move(position, *current))
            || (get_move_source(*current) == king_square && !is_legal_king_move(position, *current)))
        {
            *current = *(--movelist);
        } else {
            ++current;
            ++size;
        }
    }

    return size;
}
