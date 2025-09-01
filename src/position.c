#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "position.h"
#include "constants.h"
#include "utils.h"



Position position_from_FEN(const char* fen) {
    static const int letter_to_piece[] = {
        ['P'] = PIECE_WHITE_PAWN,   ['p'] = PIECE_BLACK_PAWN,
        ['N'] = PIECE_WHITE_KNIGHT, ['n'] = PIECE_BLACK_KNIGHT,
        ['B'] = PIECE_WHITE_BISHOP, ['b'] = PIECE_BLACK_BISHOP,
        ['R'] = PIECE_WHITE_ROOK,   ['r'] = PIECE_BLACK_ROOK,
        ['Q'] = PIECE_WHITE_QUEEN,  ['q'] = PIECE_BLACK_QUEEN,
        ['K'] = PIECE_WHITE_KING,   ['k'] = PIECE_BLACK_KING
    };

    Position position = { 0 };
    int file = FILE_A;
    int rank = RANK_8;

    /* Board. */
    for (; *fen != ' '; ++fen) {
        char c = *fen;
        if (c == '/') {
            file = FILE_A;
            --rank;
        } else if (c >= '1' && c <= '8') {
            file += c - '0';
        } else {
            position.board[letter_to_piece[(int)c]] |= COORDINATES_MASK(file++, rank);
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
        file = CHAR_TO_FILE(*fen++);
        rank = CHAR_TO_RANK(*fen++);
        position.en_passant = COORDINATES_MASK(file, rank);
    } else {
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

void position_to_FEN(Position* position, char* fen_out) {
    static const char piece_to_letter[] = {
        [PIECE_WHITE_PAWN] = 'P',   [PIECE_BLACK_PAWN] = 'p',
        [PIECE_WHITE_KNIGHT] = 'N', [PIECE_BLACK_KNIGHT] = 'n',
        [PIECE_WHITE_BISHOP] = 'B', [PIECE_BLACK_BISHOP] = 'b',
        [PIECE_WHITE_ROOK] = 'R',   [PIECE_BLACK_ROOK] = 'r',
        [PIECE_WHITE_QUEEN] = 'Q',  [PIECE_BLACK_QUEEN] = 'q',
        [PIECE_WHITE_KING] = 'K',   [PIECE_BLACK_KING] = 'k'
    };

    /* Board. */
    int empty;
    for (int rank = RANK_8; rank >= RANK_1; --rank) {
        empty = 0;
        for (int file = FILE_A; file <= FILE_H; ++file) {
            Bitboard mask = COORDINATES_MASK(file, rank);
            bool piece_found = false;
            
            for (int piece = PIECE_WHITE_PAWN; piece < PIECE_COUNT; ++piece) {
                if ((position->board[piece] & mask)) {
                    if (empty) {
                        *fen_out = '0' + (char)empty;
                        ++fen_out;
                        empty = 0;
                    }
                    *fen_out = piece_to_letter[piece];
                    ++fen_out;
                    piece_found = true;
                    break;
                }
            }

            if (!piece_found) ++empty;
        }

        if (empty) *fen_out++ = '0' + (char)empty;
        if (rank != RANK_1) *fen_out++ = '/';
    }
    *fen_out++ = ' ';

    /* Side to move. */
    *fen_out++ = (position->side_to_move == COLOR_WHITE) ? 'w' : 'b';
    *fen_out++ = ' ';

    /* Castling rights. */
    if (position->castling_rights == NO_CASTLING) {
        *fen_out++ = '-';
    } else {
        if (position->castling_rights & WHITE_00) *fen_out++ = 'K';
        if (position->castling_rights & WHITE_000) *fen_out++ = 'Q';
        if (position->castling_rights & BLACK_00) *fen_out++ = 'k';
        if (position->castling_rights & BLACK_000) *fen_out++ = 'q';
    }
    *fen_out++ = ' ';

    /* En passant. */
    if (position->en_passant) {
        int trailing_zeroes = ctzll(position->en_passant);
        int file = trailing_zeroes & 7; // Fast modulo 8.
        int rank = trailing_zeroes >> 3; // Fast divide by 8.
        *fen_out++ = 'a' + (char)file;
        *fen_out++ = '1' + (char)rank;
    } else {
        *fen_out++ = '-';
    }
    *fen_out++ = ' ';

    /* Halfmove clock. */
    fen_out += sprintf(fen_out, "%d ", position->halfmove_clock);

    /* Fullmove counter. */
    fen_out += sprintf(fen_out, "%d ", position->fullmove_counter);

    /* Null terminate. */
    *fen_out = '\0';
}
