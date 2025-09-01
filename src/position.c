#include <ctype.h>
#include <stdlib.h>

#include "position.h"
#include "constants.h"



Position position_from_FEN(const char* fen) {
    static const int letter_to_piece[] = {
        ['P'] = PIECE_WHITE_PAWN,
        ['N'] = PIECE_WHITE_KNIGHT,
        ['B'] = PIECE_WHITE_BISHOP,
        ['R'] = PIECE_WHITE_ROOK,
        ['Q'] = PIECE_WHITE_QUEEN,
        ['K'] = PIECE_WHITE_KING,

        ['p'] = PIECE_BLACK_PAWN,
        ['n'] = PIECE_BLACK_KNIGHT,
        ['b'] = PIECE_BLACK_BISHOP,
        ['r'] = PIECE_BLACK_ROOK,
        ['q'] = PIECE_BLACK_QUEEN,
        ['k'] = PIECE_BLACK_KING
    };

    Position position = { 0 };
    int file = FILE_A;
    int rank = RANK_8;
    while (*fen != ' ') {
        if (*fen == '/') {
            file = FILE_A;
            --rank;
        } else if (isdigit(*fen)) {
            file += *fen - '0';
        } else {
            position.board[letter_to_piece[(int)*fen]] |= COORDINATES_MASK(file, rank);
            ++file;
        }
        ++fen;
    }
    ++fen;

    position.side_to_move = *fen++ == 'w' ? COLOR_WHITE : COLOR_BLACK;
    ++fen;

    if (*fen == '-') {
        position.castling_rights = NO_CASTLING;
        ++fen;
    } else {
        while (*fen != ' ') {
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
                default:
                    break;
            }

            ++fen;
        }
    }
    ++fen;
    
    if (*fen != '-') {
        file = CHAR_TO_FILE(*fen++);
        rank = CHAR_TO_RANK(*fen++);

        position.en_passant = COORDINATES_MASK(file, rank);
    } else {
        ++fen;
    }
    ++fen;

    position.halfmove_clock = *fen++ - '0';
    if (*fen != ' ') position.halfmove_clock = 10 * position.halfmove_clock + *fen++ - '0';
    ++fen;
    
    position.fullmove_counter = (int)strtol(fen, NULL, 10);

    return position;
}
