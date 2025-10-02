#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "position.h"
#include "bitboard.h"
#include "move_generation.h"
#include "types.h"
#include "util.h"



const char start_position[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


static void initialise_position(Position* position) {
    assert(position != NULL);

    position->castling_squares[WHITE_00] = square_bitboard(SQUARE_F1) | square_bitboard(SQUARE_G1);
    position->castling_squares[WHITE_000] = square_bitboard(SQUARE_D1) | square_bitboard(SQUARE_C1) | square_bitboard(SQUARE_B1);
    position->castling_squares[BLACK_00] = square_bitboard(SQUARE_F8) | square_bitboard(SQUARE_G8);
    position->castling_squares[BLACK_000] = square_bitboard(SQUARE_D8) | square_bitboard(SQUARE_C8) | square_bitboard(SQUARE_B8);
    position->castling_squares[KING_SIDE] = position->castling_squares[WHITE_00] | position->castling_squares[BLACK_00];
    position->castling_squares[QUEEN_SIDE] = position->castling_squares[WHITE_000] | position->castling_squares[BLACK_000];
    position->castling_squares[WHITE_CASTLING] = position->castling_squares[WHITE_00] | position->castling_squares[WHITE_000];
    position->castling_squares[BLACK_CASTLING] = position->castling_squares[BLACK_00] | position->castling_squares[BLACK_000];
    position->castling_squares[ANY_CASTLING] = position->castling_squares[WHITE_CASTLING] | position->castling_squares[BLACK_CASTLING];
}

static Bitboard attackers_of_square(const struct Position* position, Color color, Bitboard occupancy, Square square) {
    assert(position != NULL);
    assert(is_valid_square(square));

    return ((slider_attacks(PIECE_TYPE_BISHOP, square, occupancy) & get_bishop_queen_occupancy(position, color))
          | (slider_attacks(PIECE_TYPE_ROOK, square, occupancy) & get_rook_queen_occupancy(position, color))
          | (piece_base_attack(PIECE_TYPE_KNIGHT, square) & position->board[get_piece(color, PIECE_TYPE_KNIGHT)])
          | (piece_base_attack(get_piece(!color, PIECE_TYPE_PAWN), square) & position->board[get_piece(color, PIECE_TYPE_PAWN)])
          | (piece_base_attack(PIECE_TYPE_KING, square) & position->board[get_piece(color, PIECE_TYPE_KING)]));
}

inline bool is_legal_pinned_move(const struct Position* position, Move move) {
    assert(position != NULL);
    assert(move_type(move) == MOVE_TYPE_NORMAL);

    return (line_bitboard(move_source(move), move_destination(move)) & position->board[get_piece(position->side_to_move, PIECE_TYPE_KING)]) != EMPTY_BITBOARD;
}

bool is_legal_king_move(const struct Position* position, Move move) {
    assert(position != NULL);
    assert(move_source(move) == get_king_square(position));

    if (move_type(move) == MOVE_TYPE_CASTLE) {
        CastlingRights castle_type = NO_CASTLING;
        switch (move_destination(move)) {
            case SQUARE_G1:
                castle_type = WHITE_00;
                break;
            case SQUARE_C1:
                castle_type = WHITE_000;
                break;
            case SQUARE_G8:
                castle_type = BLACK_00;
                break;
            case SQUARE_C8:
                castle_type = BLACK_000;
                break;
            default:
                assert(false);
                break;
        }

        // At this point, for castling moves, we have only checked whether there are pieces in the way and whether the king is in check.
        // We will now check if the king moves over an attacked square.
        Bitboard castling_squares = position->castling_squares[castle_type];
        while (castling_squares) {
            Square square = (Square)pop_lsb64(&castling_squares);
            if (attackers_of_square(position, !position->side_to_move, position->total_occupancy, square) != EMPTY_BITBOARD)
                return false;
        }

        return true;
    }

    return attackers_of_square(position, !position->side_to_move, position->total_occupancy, move_destination(move)) == EMPTY_BITBOARD;
}

inline Bitboard compute_checkers(const struct Position* position, Color color) {
    assert(position != NULL);
    assert(is_valid_color(color));

    Square king_square = (Square)lsb64(position->board[get_piece(color, PIECE_TYPE_KING)]);

    return attackers_of_square(position, !color, position->total_occupancy, king_square);
}

inline Bitboard compute_blockers(const struct Position* position, Color color) {
    assert(position != NULL);
    assert(is_valid_color(color));

    Square king_square = (Square)lsb64(position->board[get_piece(color, PIECE_TYPE_KING)]);

    Bitboard potential_pinners = attackers_of_square(position, !color, EMPTY_BITBOARD, king_square);

    Bitboard blockers = EMPTY_BITBOARD;

    while (potential_pinners) {
        Square pinner_square = (Square)pop_lsb64(&potential_pinners);
        Bitboard potential_blockers = between_bitboard(pinner_square, king_square) & position->total_occupancy;
    
        if (popcount64(potential_blockers) == 1)
            blockers |= potential_blockers;
    }

    return blockers;
}

char* position_to_string(Position* position, size_t* size_out) {
    assert(position != NULL);

    const int piece_to_char[] = {
        [PIECE_WHITE_PAWN] =    'P',   [PIECE_BLACK_PAWN] = 'p',
        [PIECE_WHITE_KNIGHT] =  'N', [PIECE_BLACK_KNIGHT] = 'n',
        [PIECE_WHITE_BISHOP] =  'B', [PIECE_BLACK_BISHOP] = 'b',
        [PIECE_WHITE_ROOK] =    'R',   [PIECE_BLACK_ROOK] = 'r',
        [PIECE_WHITE_QUEEN] =   'Q',  [PIECE_BLACK_QUEEN] = 'q',
        [PIECE_WHITE_KING] =    'K',   [PIECE_BLACK_KING] = 'k',

        [PIECE_NONE] = ' '
    };

    char* string = malloc(4096 * sizeof(*string));
    size_t size = (size_t)sprintf(string, "+---+---+---+---+---+---+---+---+\n");

    for (Rank rank = RANK_8; rank >= RANK_1; --rank) {
        for (File file = FILE_A; file <= FILE_H; ++file) {
            size += (size_t)sprintf(string + size, "| %c ", piece_to_char[position->pieces[coordinates_square(file, rank)]]);
        }

        size += (size_t)sprintf(string + size, "| %" PRId8 "\n+---+---+---+---+---+---+---+---+\n", rank + 1);
    }
    size += (size_t)sprintf(string + size, "  a   b   c   d   e   f   g   h\n");

    char* fen = position_to_FEN(position, NULL);
    size += (size_t)sprintf(string + size, "FEN: %s\n", fen);
    free(fen);

    string = realloc(string, size + 1); // +1 for \0.

    if (size_out != NULL)
        *size_out = size;

    return string;
}

Position position_from_FEN(const char* fen) {
    assert(fen != NULL);

    const Piece letter_to_piece[] = {
        ['P'] = PIECE_WHITE_PAWN,   ['p'] = PIECE_BLACK_PAWN,
        ['N'] = PIECE_WHITE_KNIGHT, ['n'] = PIECE_BLACK_KNIGHT,
        ['B'] = PIECE_WHITE_BISHOP, ['b'] = PIECE_BLACK_BISHOP,
        ['R'] = PIECE_WHITE_ROOK,   ['r'] = PIECE_BLACK_ROOK,
        ['Q'] = PIECE_WHITE_QUEEN,  ['q'] = PIECE_BLACK_QUEEN,
        ['K'] = PIECE_WHITE_KING,   ['k'] = PIECE_BLACK_KING
    };

    Position position = { 0 };
    initialise_position(&position);
    
    /* Board. */
    File file = FILE_A;
    Rank rank = RANK_8;
    for (; *fen != ' '; ++fen) {
        char c = *fen;
        if (c == '/') {
            file = FILE_A;
            --rank;
        } else if (c >= '1' && c <= '8') {
            Square square = coordinates_square(file, rank);
            for (size_t i = 0; i < (size_t)(c - '0'); ++i)
                position.pieces[square++] = PIECE_NONE;
            file += (File)(c - '0');
        } else {
            Piece piece = letter_to_piece[(int)c];
            position.board[piece] |= coordinates_bitboard(file, rank);
            position.pieces[coordinates_square(file, rank)] = piece;

            ++file;
        }
    }
    ++fen; // Skip space.

    /* Side to move. */
    position.side_to_move = (*fen++ == 'w') ? COLOR_WHITE : COLOR_BLACK;
    ++fen; // Skip space.

    /* Castling rights. */
    if (*fen != '-') {
        for (; *fen != ' '; ++fen) {
            switch (*fen) {
                case 'K':
                    position.castling_rights |= WHITE_00;
                    break;
                case 'Q':
                    position.castling_rights |= WHITE_000;
                    break;
                case 'k':
                    position.castling_rights |= BLACK_00;
                    break;
                case 'q':
                    position.castling_rights |= BLACK_000;
                    break;
            }
        }
    } else {
        ++fen;
    }
    ++fen; // Skip space.
    
    /* En passant. */
    if (*fen != '-') {
        file = char_to_file(*fen++);
        rank = char_to_rank(*fen++);
        position.en_passant_square = coordinates_square(file, rank);
    } else {
        position.en_passant_square = SQUARE_NONE;
        ++fen;
    }
    ++fen; // Skip space.

    /* Halfmove clock. */
    int h = 0;
    while (*fen >= '0' && *fen <= '9') h = h * 10 + (*fen++ - '0');
    position.halfmove_clock = h;
    ++fen; // Skip space.

    /* Fullmove counter. */
    h = 0;
    while (*fen >= '0' && *fen <= '9') h = h * 10 + (*fen++ - '0');
    position.fullmove_counter = h;

    position.occupancy[COLOR_WHITE] = position.board[PIECE_WHITE_PAWN]
                                    | position.board[PIECE_WHITE_KNIGHT]
                                    | position.board[PIECE_WHITE_BISHOP]
                                    | position.board[PIECE_WHITE_ROOK]
                                    | position.board[PIECE_WHITE_QUEEN]
                                    | position.board[PIECE_WHITE_KING];
    position.occupancy[COLOR_BLACK] = position.board[PIECE_BLACK_PAWN]
                                    | position.board[PIECE_BLACK_KNIGHT]
                                    | position.board[PIECE_BLACK_BISHOP]
                                    | position.board[PIECE_BLACK_ROOK]
                                    | position.board[PIECE_BLACK_QUEEN]
                                    | position.board[PIECE_BLACK_KING];
    position.total_occupancy = position.occupancy[COLOR_WHITE] | position.occupancy[COLOR_BLACK];

    position.blockers[COLOR_WHITE] = compute_blockers(&position, COLOR_WHITE);
    position.blockers[COLOR_BLACK] = compute_blockers(&position, COLOR_BLACK);
    position.checkers[COLOR_WHITE] = compute_checkers(&position, COLOR_WHITE);
    position.checkers[COLOR_BLACK] = compute_checkers(&position, COLOR_BLACK);

    return position;
}

char* position_to_FEN(Position* position, size_t* size_out) {
    assert(position != NULL);

    const char piece_to_letter[] = {
        [PIECE_WHITE_PAWN] = 'P',   [PIECE_BLACK_PAWN] = 'p',
        [PIECE_WHITE_KNIGHT] = 'N', [PIECE_BLACK_KNIGHT] = 'n',
        [PIECE_WHITE_BISHOP] = 'B', [PIECE_BLACK_BISHOP] = 'b',
        [PIECE_WHITE_ROOK] = 'R',   [PIECE_BLACK_ROOK] = 'r',
        [PIECE_WHITE_QUEEN] = 'Q',  [PIECE_BLACK_QUEEN] = 'q',
        [PIECE_WHITE_KING] = 'K',   [PIECE_BLACK_KING] = 'k'
    };

    char* fen = malloc(1024 * sizeof(*fen));
    char* current_fen = fen;

    /* Board. */
    int empty;
    for (Rank rank = RANK_8; rank >= RANK_1; --rank) {
        empty = 0;
        for (File file = FILE_A; file <= FILE_H; ++file) {
            Square square = coordinates_square(file, rank);
            
            if (position->pieces[square] == PIECE_NONE) {
                ++empty;
            } else {
                if (empty != 0) {
                    *current_fen++ = '0' + (char)empty;
                    empty = 0;
                }
                *current_fen++ = piece_to_letter[position->pieces[square]];
            }
        }

        if (empty != 0)
            *current_fen++ = '0' + (char)empty;
        if (rank != RANK_1)
            *current_fen++ = '/';
    }
    *current_fen++ = ' ';

    /* Side to move. */
    *current_fen++ = (position->side_to_move == COLOR_WHITE) ? 'w' : 'b';
    *current_fen++ = ' ';

    /* Castling rights. */
    if (position->castling_rights == NO_CASTLING) {
        *current_fen++ = '-';
    } else {
        if (position->castling_rights & WHITE_00) *current_fen++ = 'K';
        if (position->castling_rights & WHITE_000) *current_fen++ = 'Q';
        if (position->castling_rights & BLACK_00) *current_fen++ = 'k';
        if (position->castling_rights & BLACK_000) *current_fen++ = 'q';
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

    if (size_out != NULL)
        *size_out = (size_t)(current_fen - fen);

    return fen;
}
