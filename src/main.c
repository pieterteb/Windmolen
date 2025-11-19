#include <stdio.h>

#include "bitboard.h"
#include "engine.h"
#include "uci.h"
#include "zobrist.h"



int main(void) {
    initialize_bitboards();
    initialize_zobrist_keys();

    // Make sure stdtou is line buffered.
    setvbuf(stdout, NULL, _IOLBF, 0);

    struct Engine engine;
    uci_loop(&engine);

    return 0;
}
