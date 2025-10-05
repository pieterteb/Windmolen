#ifndef PERFT_H
#define PERFT_H



#include <stddef.h>
#include <string.h>

#include "move_generation.h"
#include "position.h"



static size_t perft(const struct Position* position, size_t depth) {
    if (depth == 0)
        return 1;

    size_t nodes = 0;

    Move movelist[MAX_MOVES];
    size_t move_count = generate_legal_moves(position, movelist);

    for (size_t i = 0; i < move_count; ++i) {
        Position copy = *position;
        do_move(&copy, movelist[i]);
        nodes += perft(&copy, depth - 1);
    }

    return nodes;
}



#endif /* #ifndef PERFT_H */
