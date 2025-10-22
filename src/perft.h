#ifndef WINDMOLEN_PERFT_H_
#define WINDMOLEN_PERFT_H_


#include <stddef.h>

#include "move_generation.h"
#include "position.h"



__attribute__((unused)) static size_t perft(struct Position* position, size_t depth) {
    if (depth == 0) return 1;

    Move movelist[MAX_MOVES];
    size_t move_count = generate_legal_moves(position, movelist);

    if (depth == 1) return move_count;

    size_t nodes = 0;

    for (size_t i = 0; i < move_count; ++i) {
        struct Position copy = *position;
        do_move(position, movelist[i]);
        nodes += perft(position, depth - 1);
        *position = copy;
    }

    return nodes;
}


enum {
    PERFT_CAPTURES,
    PERFT_EN_PASSANT,
    PERFT_CASTLES,
    PERFT_PROMOTIONS,
    PERFT_CHECKS,
    PERFT_DISCOVERY_CHECKS,
    PERFT_DOUBLE_CHECKS,
    PERFT_CHECKMATES,

    PERFT_COUNT
};

__attribute__((unused)) static size_t extended_perft(struct Position* position, size_t depth,
                                                     size_t extended_info[PERFT_COUNT]) {
    Move movelist[MAX_MOVES];
    size_t move_count = generate_legal_moves(position, movelist);

    if (depth == 1) {
        for (size_t i = 0; i < move_count; ++i) {
            if (piece_on_square(position, move_destination(movelist[i])) != PIECE_NONE) ++extended_info[PERFT_CAPTURES];

            if (move_type(movelist[i]) == MOVE_TYPE_EN_PASSANT) {
                ++extended_info[PERFT_CAPTURES];
                ++extended_info[PERFT_EN_PASSANT];
            }

            if (move_type(movelist[i]) == MOVE_TYPE_CASTLE) ++extended_info[PERFT_CASTLES];

            if (move_type(movelist[i]) == MOVE_TYPE_PROMOTION) ++extended_info[PERFT_PROMOTIONS];

            struct Position copy = *position;
            do_move(&copy, movelist[i]);

            if (copy.checkers[copy.side_to_move] != EMPTY_BITBOARD) {
                ++extended_info[PERFT_CHECKS];

                if ((copy.checkers[copy.side_to_move] & square_bitboard(move_destination(movelist[i])))
                    == EMPTY_BITBOARD)
                    ++extended_info[PERFT_DISCOVERY_CHECKS];
            }

            if (popcount64_greater_than_one(copy.checkers[copy.side_to_move])) ++extended_info[PERFT_DOUBLE_CHECKS];

            Move temp_movelist[MAX_MOVES];
            if (copy.checkers[copy.side_to_move] != EMPTY_BITBOARD && generate_legal_moves(&copy, temp_movelist) == 0)
                ++extended_info[PERFT_CHECKMATES];
        }

        return move_count;
    }

    size_t nodes = 0;

    for (size_t i = 0; i < move_count; ++i) {
        struct Position copy = *position;
        do_move(&copy, movelist[i]);
        nodes += extended_perft(&copy, depth - 1, extended_info);
    }

    return nodes;
}



#endif /* #ifndef WINDMOLEN_PERFT_H_ */
