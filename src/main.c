#include <stdio.h>
#include <stdlib.h>

#include "bitboard.h"
#include "perft.h"
#include "position.h"



int main(void) {
    initialize_bitboards();

    // "3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1" perft 6 = 1134888
    // "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1" perft 6 = 1015133
    // "8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1" perft 6 = 1440467
    // "5k2/8/8/8/8/8/8/4K2R w K - 0 1" perft 6 = 661072
    // "3k4/8/8/8/8/8/8/R3K3 w Q - 0 1" perft 6 = 803711
    // "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1" perft 4 = 1274206

    // clang-format off
    struct Position position = position_from_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); // Passed extended 1-6
    // struct Position position = position_from_FEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");   // Passed extended 1-4 and ~5
    // struct Position position = position_from_FEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 ");  // Passed extended 1-7
    // struct Position position = position_from_FEN("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");   // Passed extended 1-6
    // struct Position position = position_from_FEN("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");  // Passed 1-5
    // struct Position position = position_from_FEN("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");   // Passed 1-5
    // struct Position position = position_from_FEN("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1");  // Passed 1-6
    // struct Position position = position_from_FEN("8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1");     // Passed 1-6
    // struct Position position = position_from_FEN("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1");    // Passed 1-6
    // struct Position position = position_from_FEN("5k2/8/8/8/8/8/8/4K2R w K - 0 1");     // Passed 1-6
    // struct Position position = position_from_FEN("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1");    // Passed 1-6
    // struct Position position = position_from_FEN("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1");  // Passed 1-4

    for (size_t i = 1; i <= 6; ++i) {
        size_t perft_result = perft(&position, i);
        printf("Perft %zu is: %zu\n", i, perft_result);

        // size_t extended_info[PERFT_COUNT] = {0};
        // size_t perft_result = extended_perft(&position, i, extended_info);
        // printf("Perft %zu is: nodes %zu, captures %zu, en passant %zu, castles %zu, promotions %zu, checks %zu, discovery checks %zu, double checks %zu, checkmates %zu\n", i, perft_result, extended_info[PERFT_CAPTURES], extended_info[PERFT_EN_PASSANT], extended_info[PERFT_CASTLES], extended_info[PERFT_PROMOTIONS], extended_info[PERFT_CHECKS], extended_info[PERFT_DISCOVERY_CHECKS], extended_info[PERFT_DOUBLE_CHECKS], extended_info[PERFT_CHECKMATES]);
    }
    //clang-format on

    return 0;
}
