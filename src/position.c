#include <ctype.h>
#include <stdlib.h>

#include "position.h"
#include "constants.h"



Position position_from_FEN(const char* fen) {
    static const int piece_map[] = {
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

    /* Board parsing. */
    for (; *fen != ' '; ++fen) {
        char c = *fen;
        if (c == '/') {
            file = FILE_A;
            --rank;
        } else if (c >= '1' && c <= '8') {
            file += c - '0';
        } else {
            position.board[piece_map[(int)c]] |= COORDINATES_MASK(file++, rank);
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
