#include <stdio.h>

#include "bitboard.h"
#include "position.h"
#include "uci.h"



int main(void) {
    initialize_bitboards();

    uci_loop();

    // "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    // "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
    // "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 "
    // "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
    // "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"
    // "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
    // "3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1" perft 6 = 1134888
    // "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1" perft 6 = 1015133
    // "8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1" perft 6 = 1440467
    // "5k2/8/8/8/8/8/8/4K2R w K - 0 1" perft 6 = 661072
    // "3k4/8/8/8/8/8/8/R3K3 w Q - 0 1" perft 6 = 803711
    // "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1" perft 4 = 1274206


    // for (size_t i = 1; i <= 7; ++i) {
    //     size_t perft_result = perft(&main_position, i);
    //     printf("Perft %zu is: %zu\n", i, perft_result);
    // }

    char* position_string = position_to_string(&main_position, NULL);
    printf("%s\n", position_string);
    free(position_string);
    
    return 0;
}
