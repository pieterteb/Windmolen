#include <stdio.h>
#include <stdlib.h>

#include "bitboard.h"
#include "position.h"



int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i)
        printf("%s ", argv[0]);
    printf("\n");

    initialise_bitboards();

    Position position = position_from_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    char* position_string = position_to_string(&position, NULL);
    printf("%s", position_string);
    free(position_string);

    printf("\n");

    char* bitboard_string = bitboard_to_string(slider_attacks(PIECE_TYPE_BISHOP, SQUARE_E4, FILE_C_BITBOARD | RANK_5_BITBOARD), NULL);
    printf("%s", bitboard_string);
    free(bitboard_string);

    return 0;
}
