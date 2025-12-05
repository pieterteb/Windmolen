#include <stdio.h>

#include "bitboard.h"
#include "engine.h"
#include "uci.h"
#include "zobrist.h"
#include "evaluation.h"



int main(void) {
    initialize_bitboards();
    initialize_zobrist_keys();
    initialize_piece_square_tables();

    // Make sure stdout is line buffered.
    setvbuf(stdout, NULL, _IOLBF, 0);

    struct Engine engine;
    uci_loop(&engine);

    return 0;
}
