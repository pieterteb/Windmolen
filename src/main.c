#include <stdio.h>
#include <stdlib.h>

#include "bitboard.h"
#include "move_generation.h"
#include "position.h"


static const char square_to_string[64][3] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

int main(void) {
    initialise_bitboards();

    Position position = position_from_FEN("r1bqkb1r/pp1p1ppp/n1p2n2/4p3/3NP3/2NP4/PPP2PPP/R1BQKB1R b KQkq - 0 5");
    
    char* position_string = position_to_string(&position, NULL);
    printf("%s", position_string);
    free(position_string);

    printf("\n");

    Move movelist[256];
    size_t move_count = generate_pseudo_moves(&position, movelist);

    printf("%zu moves found:\n", move_count);
    for (size_t i = 0; i < move_count; ++i) {
        printf("from %s to %s\n", square_to_string[move_source(movelist[i])], square_to_string[move_destination(movelist[i])]);
    }

    // char* bitboard_string = bitboard_to_string(slider_attacks(PIECE_TYPE_BISHOP, SQUARE_E4, FILE_C_BITBOARD | RANK_5_BITBOARD), NULL);
    // printf("%s", bitboard_string);
    // free(bitboard_string);

    // char* bitboard_string = bitboard_to_string(piece_base_attack(PIECE_TYPE_KNIGHT, SQUARE_G1), NULL);
    // printf("%s", bitboard_string);
    // free(bitboard_string);

    return 0;
}
