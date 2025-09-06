#include <inttypes.h>
#include <stdio.h>

#include "bitboard.h"
#include "constants.h"


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
