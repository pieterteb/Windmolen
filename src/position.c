#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "position.h"

#include "bitboard.h"
#include "move_generation.h"
#include "types.h"
#include "util.h"



static inline Bitboard compute_checkers(const struct Position* position, Color color) {
    assert(position != NULL);
    assert(is_valid_color(color));

    return attackers_of_square(position, king_square(position, color), position->total_occupancy)
         & piece_occupancy_by_color(position, !color);
}

static inline Bitboard compute_blockers(const struct Position* position, Color color) {
    assert(position != NULL);
    assert(is_valid_color(color));

    Square king = king_square(position, color);

    Bitboard potential_pinners = attackers_of_square(position, king, EMPTY_BITBOARD)
                               & piece_occupancy_by_color(position, !color);

    Bitboard blockers = EMPTY_BITBOARD;

    while (potential_pinners != EMPTY_BITBOARD) {
        Square pinner_square        = (Square)pop_lsb64(&potential_pinners);
        Bitboard potential_blockers = between_bitboard(pinner_square, king)
                                    & (position->total_occupancy ^ square_bitboard(king));

        if (!popcount64_greater_than_one(potential_blockers)) blockers |= potential_blockers;
    }

    return blockers;
}

void do_move(struct Position* position, Move move) {
    assert(position != NULL);
    assert(is_valid_move(move));

    const Square source      = move_source(move);
    const Square destination = move_destination(move);
    const MoveType type      = move_type(move);
    Piece piece              = position->piece_on_square[source];
    const Color side_to_move = position->side_to_move;
    const Color opponent     = !side_to_move;

    const bool is_capture = position->piece_on_square[destination] != PIECE_NONE || type == MOVE_TYPE_EN_PASSANT;

    assert(piece != PIECE_NONE);
    assert(is_valid_color(side_to_move));
    assert(piece_color(piece) == side_to_move);
    assert(type != MOVE_TYPE_EN_PASSANT || position->en_passant_square != SQUARE_NONE);
    assert(popcount64(piece_occupancy(position, position->side_to_move, PIECE_TYPE_KING)) == 1);

    position->occupancy_by_piece[piece] ^= square_bitboard(source);
    position->piece_on_square[source]    = PIECE_NONE;

    if (type == MOVE_TYPE_PROMOTION) piece = get_piece(side_to_move, promotion_to_piece_type(move));

    position->occupancy_by_piece[piece] |= square_bitboard(destination);

    if (is_capture) {
        if (type == MOVE_TYPE_EN_PASSANT) {
            Square deletion_square                                              = (Square)(destination
                                              + ((side_to_move == COLOR_WHITE) ? DIRECTION_SOUTH : DIRECTION_NORTH));
            position->occupancy_by_piece[get_piece(opponent, PIECE_TYPE_PAWN)] ^= square_bitboard(deletion_square);
            position->piece_on_square[deletion_square]                          = PIECE_NONE;
        } else {
            Piece captured_piece = piece_on_square(position, destination);

            assert(piece_color(captured_piece) == opponent);

            position->occupancy_by_piece[captured_piece] ^= square_bitboard(destination);
        }
    }

    position->piece_on_square[destination] = piece;

    if (get_piece_type(piece) == PIECE_TYPE_KING) {
        position->king_square[side_to_move] = destination;

        if (type == MOVE_TYPE_CASTLE) {
            assert(!is_capture);
            assert(side_to_move == COLOR_BLACK || (destination == SQUARE_G1 || destination == SQUARE_C1));
            assert(side_to_move == COLOR_WHITE || (destination == SQUARE_G8 || destination == SQUARE_C8));

            // Squares between which the rook switches based on the destination of the king.
            // clang-format off
            static const Bitboard rook_bitboards[SQUARE_COUNT] = {
                [SQUARE_G1] = SQUARE_BITBOARD(SQUARE_F1) | SQUARE_BITBOARD(SQUARE_H1),
                [SQUARE_C1] = SQUARE_BITBOARD(SQUARE_A1) | SQUARE_BITBOARD(SQUARE_D1),
                [SQUARE_G8] = SQUARE_BITBOARD(SQUARE_F8) | SQUARE_BITBOARD(SQUARE_H8),
                [SQUARE_C8] = SQUARE_BITBOARD(SQUARE_A8) | SQUARE_BITBOARD(SQUARE_D8)
            };
            static const Square rook_sources[SQUARE_COUNT] = {
                [SQUARE_G1] = SQUARE_H1,
                [SQUARE_C1] = SQUARE_A1,
                [SQUARE_G8] = SQUARE_H8,
                [SQUARE_C8] = SQUARE_A8
            };
            static const Square rook_destinations[SQUARE_COUNT] = {
                [SQUARE_G1] = SQUARE_F1,
                [SQUARE_C1] = SQUARE_D1,
                [SQUARE_G8] = SQUARE_F8,
                [SQUARE_C8] = SQUARE_D8
            };
            // clang-format on

            Square rook_source_square      = rook_sources[destination];
            Square rook_destination_square = rook_destinations[destination];

            position->piece_on_square[rook_destination_square] = position->piece_on_square[rook_source_square];
            position->piece_on_square[rook_source_square]      = PIECE_NONE;

            // This switches the rook bit between the rook source and destination squares.
            position->occupancy_by_piece[get_piece(side_to_move, PIECE_TYPE_ROOK)] ^= rook_bitboards[destination];
        }

        position->castling_rights &= (side_to_move == COLOR_WHITE) ? ~CASTLE_WHITE : ~CASTLE_BLACK;
    }

    bool is_double_pawn_push = get_piece_type(piece) == PIECE_TYPE_PAWN
                            && abs(destination - source) == 2 * DIRECTION_NORTH;
    if (is_double_pawn_push)
        position->en_passant_square = (Square)(source
                                               + ((side_to_move == COLOR_WHITE) ? DIRECTION_NORTH : DIRECTION_SOUTH));
    else
        position->en_passant_square = SQUARE_NONE;

    position->occupancy_by_color[COLOR_WHITE] = position->occupancy_by_piece[PIECE_WHITE_PAWN]
                                              | position->occupancy_by_piece[PIECE_WHITE_KNIGHT]
                                              | position->occupancy_by_piece[PIECE_WHITE_BISHOP]
                                              | position->occupancy_by_piece[PIECE_WHITE_ROOK]
                                              | position->occupancy_by_piece[PIECE_WHITE_QUEEN]
                                              | position->occupancy_by_piece[PIECE_WHITE_KING];
    position->occupancy_by_color[COLOR_BLACK] = position->occupancy_by_piece[PIECE_BLACK_PAWN]
                                              | position->occupancy_by_piece[PIECE_BLACK_KNIGHT]
                                              | position->occupancy_by_piece[PIECE_BLACK_BISHOP]
                                              | position->occupancy_by_piece[PIECE_BLACK_ROOK]
                                              | position->occupancy_by_piece[PIECE_BLACK_QUEEN]
                                              | position->occupancy_by_piece[PIECE_BLACK_KING];

    position->occupancy_by_type[PIECE_TYPE_PAWN] = position->occupancy_by_piece[PIECE_WHITE_PAWN]
                                                 | position->occupancy_by_piece[PIECE_BLACK_PAWN];
    position->occupancy_by_type[PIECE_TYPE_KNIGHT] = position->occupancy_by_piece[PIECE_WHITE_KNIGHT]
                                                   | position->occupancy_by_piece[PIECE_BLACK_KNIGHT];
    position->occupancy_by_type[PIECE_TYPE_BISHOP] = position->occupancy_by_piece[PIECE_WHITE_BISHOP]
                                                   | position->occupancy_by_piece[PIECE_BLACK_BISHOP];
    position->occupancy_by_type[PIECE_TYPE_ROOK] = position->occupancy_by_piece[PIECE_WHITE_ROOK]
                                                 | position->occupancy_by_piece[PIECE_BLACK_ROOK];
    position->occupancy_by_type[PIECE_TYPE_QUEEN] = position->occupancy_by_piece[PIECE_WHITE_QUEEN]
                                                  | position->occupancy_by_piece[PIECE_BLACK_QUEEN];
    position->occupancy_by_type[PIECE_TYPE_KING] = position->occupancy_by_piece[PIECE_WHITE_KING]
                                                 | position->occupancy_by_piece[PIECE_BLACK_KING];

    position->total_occupancy = position->occupancy_by_color[COLOR_WHITE] | position->occupancy_by_color[COLOR_BLACK];

    if (position->piece_on_square[SQUARE_A1] != PIECE_WHITE_ROOK) position->castling_rights &= ~CASTLE_WHITE_000;
    if (position->piece_on_square[SQUARE_H1] != PIECE_WHITE_ROOK) position->castling_rights &= ~CASTLE_WHITE_00;
    if (position->piece_on_square[SQUARE_A8] != PIECE_BLACK_ROOK) position->castling_rights &= ~CASTLE_BLACK_000;
    if (position->piece_on_square[SQUARE_H8] != PIECE_BLACK_ROOK) position->castling_rights &= ~CASTLE_BLACK_00;

    position->side_to_move = opponent;
    if (side_to_move == COLOR_BLACK) ++position->fullmove_counter;

    position->checkers[opponent]     = compute_checkers(position, opponent);
    position->blockers[side_to_move] = compute_blockers(position, side_to_move);
    position->blockers[opponent]     = compute_blockers(position, opponent);

    if (is_capture || get_piece_type(piece) == PIECE_TYPE_PAWN)
        position->halfmove_clock = 0;
    else
        ++position->halfmove_clock;
}


char* position_to_string(const struct Position* position, size_t* size_out) {
    assert(position != NULL);

    // clang-format off
    const int piece_to_char[] = {
        [PIECE_WHITE_PAWN]   = 'P', [PIECE_BLACK_PAWN]   = 'p',
        [PIECE_WHITE_KNIGHT] = 'N', [PIECE_BLACK_KNIGHT] = 'n',
        [PIECE_WHITE_BISHOP] = 'B', [PIECE_BLACK_BISHOP] = 'b',
        [PIECE_WHITE_ROOK]   = 'R', [PIECE_BLACK_ROOK]   = 'r',
        [PIECE_WHITE_QUEEN]  = 'Q', [PIECE_BLACK_QUEEN]  = 'q',
        [PIECE_WHITE_KING]   = 'K', [PIECE_BLACK_KING]   = 'k',

        [PIECE_NONE] = ' '
    };
    // clang-format on

    char* string = malloc(4096 * sizeof(*string));
    size_t size  = (size_t)sprintf(string, "+---+---+---+---+---+---+---+---+\n");

    for (Rank rank = RANK_8; rank >= RANK_1; --rank) {
        for (File file = FILE_A; file <= FILE_H; ++file) {
            size += (size_t)sprintf(string + size,
                                    "| %c ",
                                    piece_to_char[piece_on_square(position, coordinate_square(file, rank))]);
        }

        size += (size_t)sprintf(string + size, "| %" PRId8 "\n+---+---+---+---+---+---+---+---+\n", rank + 1);
    }
    size += (size_t)sprintf(string + size, "  a   b   c   d   e   f   g   h\n");

    char* fen  = position_to_FEN(position, NULL);
    size      += (size_t)sprintf(string + size, "FEN: %s\n", fen);
    free(fen);

    string = realloc(string, size + 1); // +1 for \0.

    if (size_out != NULL) *size_out = size;

    return string;
}

void position_from_startpos(struct Position* position) {
    position_from_FEN(position, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

const char* position_from_FEN(struct Position* position, const char* fen) {
    assert(fen != NULL);

    // clang-format off
    const Piece letter_to_piece[] = {
        ['P'] = PIECE_WHITE_PAWN,   ['p'] = PIECE_BLACK_PAWN,
        ['N'] = PIECE_WHITE_KNIGHT, ['n'] = PIECE_BLACK_KNIGHT,
        ['B'] = PIECE_WHITE_BISHOP, ['b'] = PIECE_BLACK_BISHOP,
        ['R'] = PIECE_WHITE_ROOK,   ['r'] = PIECE_BLACK_ROOK,
        ['Q'] = PIECE_WHITE_QUEEN,  ['q'] = PIECE_BLACK_QUEEN,
        ['K'] = PIECE_WHITE_KING,   ['k'] = PIECE_BLACK_KING
    };
    // clang-format on

    *position = (struct Position){0};

    /* Board. */
    File file = FILE_A;
    Rank rank = RANK_8;
    for (; *fen != ' '; ++fen) {
        char c = *fen;
        if (c == '/') {
            file = FILE_A;
            --rank;
        } else if (c >= '1' && c <= '8') {
            Square square = coordinate_square(file, rank);
            for (size_t i = 0; i < (size_t)(c - '0'); ++i)
                position->piece_on_square[square++] = PIECE_NONE;
            file += (File)(c - '0');
        } else {
            Piece piece                                              = letter_to_piece[(int)c];
            position->occupancy_by_piece[piece]                      |= coordinate_bitboard(file, rank);
            position->piece_on_square[coordinate_square(file, rank)]  = piece;

            if (piece == PIECE_WHITE_KING) position->king_square[COLOR_WHITE] = coordinate_square(file, rank);
            if (piece == PIECE_BLACK_KING) position->king_square[COLOR_BLACK] = coordinate_square(file, rank);

            ++file;
        }
    }
    ++fen; // Skip space->

    /* Side to move-> */
    position->side_to_move = (*fen++ == 'w') ? COLOR_WHITE : COLOR_BLACK;
    ++fen; // Skip space->

    /* Castling rights-> */
    if (*fen != '-') {
        for (; *fen != ' '; ++fen) {
            switch (*fen) {
                case 'K':
                    position->castling_rights |= CASTLE_WHITE_00;
                    break;
                case 'Q':
                    position->castling_rights |= CASTLE_WHITE_000;
                    break;
                case 'k':
                    position->castling_rights |= CASTLE_BLACK_00;
                    break;
                case 'q':
                    position->castling_rights |= CASTLE_BLACK_000;
                    break;
            }
        }
    } else {
        ++fen;
    }
    ++fen; // Skip space->

    /* En passant-> */
    if (*fen != '-') {
        file                       = char_to_file(*fen++);
        rank                       = char_to_rank(*fen++);
        position->en_passant_square = coordinate_square(file, rank);
    } else {
        position->en_passant_square = SQUARE_NONE;
        ++fen;
    }
    ++fen; // Skip space->

    /* Halfmove clock-> */
    int h = 0;
    while (*fen >= '0' && *fen <= '9')
        h = h * 10 + (*fen++ - '0');
    position->halfmove_clock = h;
    ++fen; // Skip space->

    /* Fullmove counter-> */
    h = 0;
    while (*fen >= '0' && *fen <= '9')
        h = h * 10 + (*fen++ - '0');
    position->fullmove_counter = h;

    position->occupancy_by_color[COLOR_WHITE] = position->occupancy_by_piece[PIECE_WHITE_PAWN]
                                             | position->occupancy_by_piece[PIECE_WHITE_KNIGHT]
                                             | position->occupancy_by_piece[PIECE_WHITE_BISHOP]
                                             | position->occupancy_by_piece[PIECE_WHITE_ROOK]
                                             | position->occupancy_by_piece[PIECE_WHITE_QUEEN]
                                             | position->occupancy_by_piece[PIECE_WHITE_KING];
    position->occupancy_by_color[COLOR_BLACK] = position->occupancy_by_piece[PIECE_BLACK_PAWN]
                                             | position->occupancy_by_piece[PIECE_BLACK_KNIGHT]
                                             | position->occupancy_by_piece[PIECE_BLACK_BISHOP]
                                             | position->occupancy_by_piece[PIECE_BLACK_ROOK]
                                             | position->occupancy_by_piece[PIECE_BLACK_QUEEN]
                                             | position->occupancy_by_piece[PIECE_BLACK_KING];

    position->occupancy_by_type[PIECE_TYPE_PAWN] = position->occupancy_by_piece[PIECE_WHITE_PAWN]
                                                | position->occupancy_by_piece[PIECE_BLACK_PAWN];
    position->occupancy_by_type[PIECE_TYPE_KNIGHT] = position->occupancy_by_piece[PIECE_WHITE_KNIGHT]
                                                  | position->occupancy_by_piece[PIECE_BLACK_KNIGHT];
    position->occupancy_by_type[PIECE_TYPE_BISHOP] = position->occupancy_by_piece[PIECE_WHITE_BISHOP]
                                                  | position->occupancy_by_piece[PIECE_BLACK_BISHOP];
    position->occupancy_by_type[PIECE_TYPE_ROOK] = position->occupancy_by_piece[PIECE_WHITE_ROOK]
                                                | position->occupancy_by_piece[PIECE_BLACK_ROOK];
    position->occupancy_by_type[PIECE_TYPE_QUEEN] = position->occupancy_by_piece[PIECE_WHITE_QUEEN]
                                                 | position->occupancy_by_piece[PIECE_BLACK_QUEEN];
    position->occupancy_by_type[PIECE_TYPE_KING] = position->occupancy_by_piece[PIECE_WHITE_KING]
                                                | position->occupancy_by_piece[PIECE_BLACK_KING];

    position->total_occupancy = position->occupancy_by_color[COLOR_WHITE] | position->occupancy_by_color[COLOR_BLACK];

    position->blockers[COLOR_WHITE] = compute_blockers(position, COLOR_WHITE);
    position->blockers[COLOR_BLACK] = compute_blockers(position, COLOR_BLACK);
    position->checkers[COLOR_WHITE] = compute_checkers(position, COLOR_WHITE);
    position->checkers[COLOR_BLACK] = compute_checkers(position, COLOR_BLACK);

    return fen;
}

char* position_to_FEN(const struct Position* position, size_t* size_out) {
    assert(position != NULL);

    // clang-format off
    const char piece_to_letter[] = {
        [PIECE_WHITE_PAWN]   = 'P', [PIECE_BLACK_PAWN]   = 'p',
        [PIECE_WHITE_KNIGHT] = 'N', [PIECE_BLACK_KNIGHT] = 'n',
        [PIECE_WHITE_BISHOP] = 'B', [PIECE_BLACK_BISHOP] = 'b',
        [PIECE_WHITE_ROOK]   = 'R', [PIECE_BLACK_ROOK]   = 'r',
        [PIECE_WHITE_QUEEN]  = 'Q', [PIECE_BLACK_QUEEN]  = 'q',
        [PIECE_WHITE_KING]   = 'K', [PIECE_BLACK_KING]   = 'k'
    };
    // clang-format on

    char* fen         = malloc(1024 * sizeof(*fen));
    char* current_fen = fen;

    /* Board. */
    int empty;
    for (Rank rank = RANK_8; rank >= RANK_1; --rank) {
        empty = 0;
        for (File file = FILE_A; file <= FILE_H; ++file) {
            Square square = coordinate_square(file, rank);

            if (piece_on_square(position, square) == PIECE_NONE) {
                ++empty;
            } else {
                if (empty != 0) {
                    *current_fen++ = '0' + (char)empty;
                    empty          = 0;
                }
                *current_fen++ = piece_to_letter[piece_on_square(position, square)];
            }
        }

        if (empty != 0) *current_fen++ = '0' + (char)empty;
        if (rank != RANK_1) *current_fen++ = '/';
    }
    *current_fen++ = ' ';

    /* Side to move. */
    *current_fen++ = (position->side_to_move == COLOR_WHITE) ? 'w' : 'b';
    *current_fen++ = ' ';

    /* Castling rights. */
    if (position->castling_rights == CASTLE_NONE) {
        *current_fen++ = '-';
    } else {
        if (position->castling_rights & CASTLE_WHITE_00) *current_fen++ = 'K';
        if (position->castling_rights & CASTLE_WHITE_000) *current_fen++ = 'Q';
        if (position->castling_rights & CASTLE_BLACK_00) *current_fen++ = 'k';
        if (position->castling_rights & CASTLE_BLACK_000) *current_fen++ = 'q';
    }
    *current_fen++ = ' ';

    /* En passant. */
    if (position->en_passant_square != SQUARE_NONE) {
        *current_fen++ = 'a' + (char)file_from_square(position->en_passant_square);
        *current_fen++ = '1' + (char)rank_from_square(position->en_passant_square);
    } else {
        *current_fen++ = '-';
    }
    *current_fen++ = ' ';

    /* Halfmove clock. */
    current_fen += sprintf(current_fen, "%d ", position->halfmove_clock);

    /* Fullmove counter. */
    current_fen += sprintf(current_fen, "%d ", position->fullmove_counter);

    /* Null terminate. */
    *current_fen = '\0';

    if (size_out != NULL) *size_out = (size_t)(current_fen - fen);

    return fen;
}
