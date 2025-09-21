#include <stdio.h>
#include <stdlib.h>

#include "bitboard.h"
#include "position.h"



int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i)
        printf("%s ", argv[0]);
    printf("\n");

    initialise_bitboards();

    Position position = position_from_FEN("3Q4/1Q4Q1/4Q3/2Q4R/Q4Q2/3Q4/1Q4Rp/1K1BBNNk w - - 0 1");

    
    print_position(stdout, &position);

    char* bitboard = bitboard_to_string(slider_attacks(PIECE_TYPE_BISHOP, SQUARE_E4, FILE_C_BITBOARD | RANK_5_BITBOARD), NULL);
    printf("%s", bitboard);
    free(bitboard);

    return 0;
}
