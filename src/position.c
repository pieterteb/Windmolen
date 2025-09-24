#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitboard.h"
#include "position.h"
#include "types.h"
#include "util.h"



const char start_position[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


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
