#include <inttypes.h>
#include <stdio.h>

#include "bitboard.h"
#include "types.h"
#include "utils.h"



void print_bitboard(FILE* stream, const Bitboard bitboard) {
    for (int rank = RANK_8; rank >= RANK_1; --rank) {
        fprintf(stream, "%d  ", rank + 1);
        for (int file = FILE_A; file < FILE_COUNT; ++file) {
            if ((bitboard & COORDINATES_MASK(file, rank)) == 0)
                fprintf(stream, " 0");
            else
                fprintf(stream, " 1");
        }
        fputc('\n', stream);
    }

    fprintf(stream, "\n    a b c d e f g h\n"

                    "\nDecimal value:     %" PRIu64
                    "\nHexadecimal value: %#018" PRIX64,
                    bitboard,
                    bitboard);
}
