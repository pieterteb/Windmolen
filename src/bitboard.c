#include <inttypes.h>
#include <stdio.h>

#include "bitboard.h"
#include "types.h"



Bitboard piece_base_attacks[PIECE_TYPE_COUNT][SQUARE_COUNT] = { 0 };


static Bitboard step_safe(Square square, Direction step) {
    Square to = square + (Square)step;
    return IS_VALID_SQUARE(to) ? SQUARE_BITBOARD(to) : BITBOARD_EMPTY;
}


void initialise_bitboards() {
    for (Square square = SQUARE_A1; square <= SQUARE_H8; ++square) {
        Bitboard square_bitboard = SQUARE_BITBOARD(square);

        piece_base_attacks[PIECE_WHITE_PAWN][square] = PAWN_ATTACKS_BITBOARD(square_bitboard, COLOR_WHITE);
        piece_base_attacks[PIECE_BLACK_PAWN][square] = PAWN_ATTACKS_BITBOARD(square_bitboard, COLOR_BLACK);

        const Direction knight_steps[8] = {
            DIRECTION_NORTH + DIRECTION_NORTHEAST, DIRECTION_EAST  + DIRECTION_NORTHEAST,
            DIRECTION_EAST  + DIRECTION_SOUTHEAST, DIRECTION_SOUTH + DIRECTION_SOUTHEAST,
            DIRECTION_SOUTH + DIRECTION_SOUTHWEST, DIRECTION_WEST  + DIRECTION_SOUTHWEST,
            DIRECTION_WEST  + DIRECTION_NORTHWEST, DIRECTION_NORTH + DIRECTION_NORTHWEST
        };
        const Direction king_steps[8] = {
            DIRECTION_NORTH, DIRECTION_NORTHEAST,
            DIRECTION_EAST,  DIRECTION_SOUTHEAST,
            DIRECTION_SOUTH, DIRECTION_SOUTHWEST,
            DIRECTION_WEST,  DIRECTION_NORTHWEST
        };
        for (size_t i = 0; i < 8; ++i) {
            piece_base_attacks[PIECE_TYPE_KNIGHT][square] |= step_safe(square, knight_steps[i]);
            piece_base_attacks[PIECE_TYPE_KING][square] |= step_safe(square, king_steps[i]);
        }

        

        piece_base_attacks[PIECE_TYPE_QUEEN][square] = piece_base_attacks[PIECE_TYPE_BISHOP][square] | piece_base_attacks[PIECE_TYPE_ROOK][square];
    }
}

void print_bitboard(FILE* stream, const Bitboard bitboard) {
    for (Rank rank = RANK_8; rank <= RANK_8; --rank) {
        fprintf(stream, "%d  ", rank + 1);
        for (File file = FILE_A; file <= FILE_H; ++file) {
            if ((bitboard & coordinates_mask(file, rank)) == 0)
                fprintf(stream, " 0");
            else
                fprintf(stream, " 1");
        }
        fputc('\n', stream);
    }

    fprintf(stream, "\n    a b c d e f g h\n"

                    "\nDecimal value:     %" PRIu64
                    "\nHexadecimal value: %#018" PRIx64,
                    bitboard,
                    bitboard);
}
