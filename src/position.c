#include "position.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitboard.h"
#include "move_generation.h"
#include "types.h"
#include "util.h"



static const char start_position_fen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
static const char kiwipete_fen[]       = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";


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

        if (!popcount64_greater_than_one(potential_blockers))
            blockers |= potential_blockers;
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
    const Color opponent     = opposite_color(side_to_move);

    assert(piece != PIECE_NONE);
    assert(is_valid_color(side_to_move));
    assert(color_of_piece(piece) == side_to_move);
    assert(type != MOVE_TYPE_EN_PASSANT || position->en_passant_square != SQUARE_NONE);
    assert(popcount64(piece_occupancy(position, side_to_move, PIECE_TYPE_KING)) == 1);

    remove_piece(position, piece, source);

    if (type == MOVE_TYPE_PROMOTION)
        piece = create_piece(side_to_move, promotion_piece_type(move));

    const PieceType piece_type          = type_of_piece(piece);
    const Bitboard destination_bitboard = square_bitboard(destination);

    const bool is_capture = position->piece_on_square[destination] != PIECE_NONE || type == MOVE_TYPE_EN_PASSANT;
    if (is_capture) {
        if (type != MOVE_TYPE_EN_PASSANT) {
            Piece captured_piece = piece_on_square(position, destination);

            assert(color_of_piece(captured_piece) == opponent);

            position->occupancy_by_type[type_of_piece(captured_piece)] ^= destination_bitboard;
            position->occupancy_by_color[opponent] ^= destination_bitboard;
        } else {
            Square deletion_square = (Square)(destination
                                              + ((side_to_move == COLOR_WHITE) ? DIRECTION_SOUTH : DIRECTION_NORTH));
            remove_piece_type(position, opponent, PIECE_TYPE_PAWN, deletion_square);
        }
    }

    position->occupancy_by_type[piece_type] |= destination_bitboard;
    position->occupancy_by_color[side_to_move] |= destination_bitboard;
    position->piece_on_square[destination] = piece;

    if (piece_type == PIECE_TYPE_KING) {
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

            const Square rook_source_square      = rook_sources[destination];
            const Square rook_destination_square = rook_destinations[destination];

            position->piece_on_square[rook_destination_square] = create_piece(side_to_move, PIECE_TYPE_ROOK);
            position->piece_on_square[rook_source_square]      = PIECE_NONE;

            // This switches the rook bit between the rook source and destination squares.
            position->occupancy_by_type[PIECE_TYPE_ROOK] ^= rook_bitboards[destination];
            position->occupancy_by_color[side_to_move] ^= rook_bitboards[destination];
        }

        position->castling_rights &= (side_to_move == COLOR_WHITE) ? ~CASTLE_WHITE : ~CASTLE_BLACK;
    }

    bool is_double_pawn_push = piece_type == PIECE_TYPE_PAWN && abs(destination - source) == 2 * DIRECTION_NORTH;
    if (is_double_pawn_push)
        position->en_passant_square = (Square)(source
                                               + ((side_to_move == COLOR_WHITE) ? DIRECTION_NORTH : DIRECTION_SOUTH));
    else
        position->en_passant_square = SQUARE_NONE;

    position->total_occupancy = position->occupancy_by_color[COLOR_WHITE] | position->occupancy_by_color[COLOR_BLACK];

    if (position->piece_on_square[SQUARE_A1] != PIECE_WHITE_ROOK)
        position->castling_rights &= ~CASTLE_WHITE_000;
    if (position->piece_on_square[SQUARE_H1] != PIECE_WHITE_ROOK)
        position->castling_rights &= ~CASTLE_WHITE_00;
    if (position->piece_on_square[SQUARE_A8] != PIECE_BLACK_ROOK)
        position->castling_rights &= ~CASTLE_BLACK_000;
    if (position->piece_on_square[SQUARE_H8] != PIECE_BLACK_ROOK)
        position->castling_rights &= ~CASTLE_BLACK_00;

    position->checkers[opponent]     = compute_checkers(position, opponent);
    position->blockers[side_to_move] = compute_blockers(position, side_to_move);
    position->blockers[opponent]     = compute_blockers(position, opponent);

    if (side_to_move == COLOR_BLACK)
        ++position->fullmove_counter;
    position->side_to_move = opponent;

    if (is_capture || piece_type == PIECE_TYPE_PAWN)
        position->halfmove_clock = 0;
    else
        ++position->halfmove_clock;
}


void setup_start_position(struct Position* position) {
    assert(position != NULL);

    setup_position_from_fen(position, start_position_fen);
}

void setup_kiwipete_position(struct Position* position) {
    assert(position != NULL);

    setup_position_from_fen(position, kiwipete_fen);
}

const char* setup_position_from_fen(struct Position* position, const char* fen) {
    assert(position != NULL);
    assert(fen != NULL);

    // clang-format off
    static const Piece char_to_piece[] = {
        ['P'] = PIECE_WHITE_PAWN,   ['p'] = PIECE_BLACK_PAWN,
        ['N'] = PIECE_WHITE_KNIGHT, ['n'] = PIECE_BLACK_KNIGHT,
        ['B'] = PIECE_WHITE_BISHOP, ['b'] = PIECE_BLACK_BISHOP,
        ['R'] = PIECE_WHITE_ROOK,   ['r'] = PIECE_BLACK_ROOK,
        ['Q'] = PIECE_WHITE_QUEEN,  ['q'] = PIECE_BLACK_QUEEN,
        ['K'] = PIECE_WHITE_KING,   ['k'] = PIECE_BLACK_KING
    };
    // clang-format on

    // piece_on_square must be the last member of struct Position in order for the memset calls to be valid.
    static_assert(offsetof(struct Position, piece_on_square) + sizeof(((struct Position*)0)->piece_on_square)
                  == sizeof(struct Position),
                  "piece_on_square must be the last member of struct Position.");
    memset(position, 0, offsetof(struct Position, piece_on_square));
    memset(&position->piece_on_square, PIECE_NONE, sizeof(position->piece_on_square));

    // Parse board configuration.
    Square square = SQUARE_A8;
    char current_char;
    while (!isspace(*fen)) {
        current_char = *fen++;
        if (isdigit(current_char)) {
            square += (Square)(current_char - '0') * DIRECTION_EAST;
        } else if (current_char == '/') {
            square += 2 * DIRECTION_SOUTH;
        } else {
            Piece piece = char_to_piece[(int)current_char];
            place_piece(position, piece, square);

            if (piece == PIECE_WHITE_KING)
                position->king_square[COLOR_WHITE] = square;
            if (piece == PIECE_BLACK_KING)
                position->king_square[COLOR_BLACK] = square;

            square += DIRECTION_EAST;
        }
    }
    ++fen;  // Skip space.

    // Parse side to move.
    position->side_to_move = (*fen++ == 'w') ? COLOR_WHITE : COLOR_BLACK;
    ++fen;  // Skip space.

    // Parse castling rights.
    do {
        switch (*fen++) {
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
            case '-':
                // Do nothing.
                break;
        }
    } while (!isspace(*fen));
    ++fen;  // Skip space.

    // Parse en passant square.
    if (*fen != '-') {
        File file                   = char_to_file(*fen++);
        Rank rank                   = char_to_rank(*fen++);
        position->en_passant_square = square_from_coordinates(file, rank);
    } else {
        position->en_passant_square = SQUARE_NONE;
        ++fen;
    }
    ++fen;  // Skip space.

    // Parse halfmove clock.
    size_t temp = 0;
    while (isdigit(*fen))
        temp = temp * 10 + (size_t)(*fen++ - '0');
    position->halfmove_clock = temp;
    ++fen;  // Skip space.

    // Parse fullmove counter.
    temp = 0;
    while (isdigit(*fen))
        temp = temp * 10 + (size_t)(*fen++ - '0');
    position->fullmove_counter = temp;

    // Compute remaining tables.
    position->total_occupancy = position->occupancy_by_color[COLOR_WHITE] | position->occupancy_by_color[COLOR_BLACK];

    position->blockers[COLOR_WHITE] = compute_blockers(position, COLOR_WHITE);
    position->blockers[COLOR_BLACK] = compute_blockers(position, COLOR_BLACK);
    position->checkers[COLOR_WHITE] = compute_checkers(position, COLOR_WHITE);
    position->checkers[COLOR_BLACK] = compute_checkers(position, COLOR_BLACK);

    return fen;
}


// clang-format off
static const int piece_to_char[] = {
    [PIECE_WHITE_PAWN]   = 'P', [PIECE_BLACK_PAWN]   = 'p',
    [PIECE_WHITE_KNIGHT] = 'N', [PIECE_BLACK_KNIGHT] = 'n',
    [PIECE_WHITE_BISHOP] = 'B', [PIECE_BLACK_BISHOP] = 'b',
    [PIECE_WHITE_ROOK]   = 'R', [PIECE_BLACK_ROOK]   = 'r',
    [PIECE_WHITE_QUEEN]  = 'Q', [PIECE_BLACK_QUEEN]  = 'q',
    [PIECE_WHITE_KING]   = 'K', [PIECE_BLACK_KING]   = 'k',

    [PIECE_NONE] = ' '
};
// clang-format on

void print_fen(const struct Position* position) {
    assert(position != NULL);

    // Print board.
    size_t empty_squares;
    for (Rank rank = RANK_8; rank >= RANK_1; --rank) {
        empty_squares = 0;
        for (File file = FILE_A; file <= FILE_H; ++file) {
            Square square = square_from_coordinates(file, rank);

            if (piece_on_square(position, square) == PIECE_NONE) {
                ++empty_squares;
            } else {
                if (empty_squares != 0) {
                    putchar('0' + (char)empty_squares);
                    empty_squares = 0;
                }
                putchar(piece_to_char[piece_on_square(position, square)]);
            }
        }

        if (empty_squares != 0)
            putchar('0' + (char)empty_squares);
        if (rank != RANK_1)
            putchar('/');
    }

    // Print side to move.
    printf((position->side_to_move == COLOR_WHITE) ? " w " : " b ");

    // Print castling rights.
    if (position->castling_rights == CASTLE_NONE) {
        printf("- ");
    } else {
        if (position->castling_rights & CASTLE_WHITE_00)
            putchar('K');
        if (position->castling_rights & CASTLE_WHITE_000)
            putchar('Q');
        if (position->castling_rights & CASTLE_BLACK_00)
            putchar('k');
        if (position->castling_rights & CASTLE_BLACK_000)
            putchar('q');
        putchar(' ');
    }

    // Print en passant square.
    if (position->en_passant_square != SQUARE_NONE) {
        putchar('a' + (char)file_from_square(position->en_passant_square));
        putchar('1' + (char)file_from_square(position->en_passant_square));
    } else {
        putchar('-');
    }

    // Print halfmove clock and fullmove counter.
    printf(" %zu %zu ", position->halfmove_clock, position->fullmove_counter);
}

void print_position(const struct Position* position) {
    assert(position != NULL);

    puts("+---+---+---+---+---+---+---+---+");

    for (Rank rank = RANK_8; rank >= RANK_1; --rank) {
        for (File file = FILE_A; file <= FILE_H; ++file)
            printf("| %c ", piece_to_char[piece_on_square(position, square_from_coordinates(file, rank))]);

        static_assert(IS_SAME_TYPE(Rank, int8_t), "Wrong format specifier used.");
        printf("| %" PRId8 "\n+---+---+---+---+---+---+---+---+\n", (Rank)(rank + 1));
    }

    puts(
    "  a   b   c   d   e   f   g   h\n"
    "FEN: ");
    print_fen(position);
    putchar('\n');
}
