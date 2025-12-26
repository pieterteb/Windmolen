#ifndef WINDMOLEN_PERFT_H_
#define WINDMOLEN_PERFT_H_


#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "move.h"
#include "move_generation.h"
#include "position.h"
#include "uci.h"
#include "util.h"



// Computes the number of leaf nodes in the chess search tree at nonzero `depth` in `position`.
static size_t perft_nonzero_depth(struct Position* position, const size_t depth) {
    assert(position != nullptr);
    assert(depth > 0);

    Move movelist[MAX_MOVES];
    const size_t move_count = generate_legal_moves(position, movelist);

    if (depth == 1)
        return move_count;

    size_t nodes = 0;
    struct PositionInfo position_info;
    for (size_t i = 0; i < move_count; ++i) {
        do_move(position, &position_info, movelist[i]);
        nodes += perft_nonzero_depth(position, depth - 1);
        undo_move(position, movelist[i]);
    }

    return nodes;
}

// Computes the number of leaf nodes in the chess search tree at `depth` in `position`.
static INLINE size_t perft(struct Position* position, const size_t depth) {
    assert(position != nullptr);

    if (depth == 0)
        return 1;

    return perft_nonzero_depth(position, depth);
}

// Computes the number of leaf nodes in the chess search tree at nonzero `depth` in `position`. Additionally, prints the
// number of leaf nodes in the chess search tree for each legal move in `position`.
static size_t divide(struct Position* position, const size_t depth) {
    assert(position != nullptr);
    assert(depth > 0);

    Move movelist[MAX_MOVES];
    const size_t move_count = generate_legal_moves(position, movelist);

    size_t nodes = 0;
    size_t move_nodes;
    struct PositionInfo position_info;
    for (size_t i = 0; i < move_count; ++i) {
        do_move(position, &position_info, movelist[i]);
        move_nodes = perft(position, depth - 1);
        undo_move(position, movelist[i]);

        nodes += move_nodes;

        print_move(movelist[i]);
        printf(": %zu\n", move_nodes);
    }

    return nodes;
}


// Struct to keep data of extended perft. To clarify, a check can only belong to one category of checks and a mating
// move can only belong to one category of mates. This is the same as done by The Grand Chess Tree:
// https://grandchesstree.com/
struct ExtendedPerft {
    size_t captures;
    size_t en_passants;
    size_t castles;
    size_t promotions;
    size_t direct_checks;
    size_t single_discovered_checks;
    size_t direct_discovered_checks;
    size_t double_discovered_checks;
    size_t direct_mates;
    size_t single_discovered_mates;
    size_t direct_discovered_mates;
    size_t double_discovered_mates;
};

// Same as perft_depth_nonzero(), except it computes extra information and stores that in `ext_perft`.
static size_t extended_perft_nonzero_depth(struct Position* position, size_t depth, struct ExtendedPerft* ext_perft) {
    assert(position != nullptr);
    assert(ext_perft != nullptr);
    assert(depth > 0);

    Move movelist[MAX_MOVES];
    const size_t move_count = generate_legal_moves(position, movelist);

    if (depth == 1) {
        for (size_t i = 0; i < move_count; ++i) {
            const enum Square destination = move_destination(movelist[i]);
            const enum MoveType move_type = type_of_move(movelist[i]);

            // We do not test for en passant here as that will be done later.
            if (piece_on_square(position, destination) != PIECE_NONE)
                ++ext_perft->captures;

            // These three cases are mutually exclusive.
            if (move_type == MOVE_TYPE_EN_PASSANT) {
                // An en passant move is always a capture.
                ++ext_perft->captures;
                ++ext_perft->en_passants;
            } else if (move_type == MOVE_TYPE_CASTLE) {
                ++ext_perft->castles;
            } else if (move_type == MOVE_TYPE_PROMOTION) {
                ++ext_perft->promotions;
            }


            const bool direct_check     = gives_direct_check(position, movelist[i]);
            const bool discovered_check = gives_discovered_check(position, movelist[i]);

            // We only look for checkmate if the move is check. This saves computation time as we do not need to
            // generate all legal moves all the time.
            if (direct_check | discovered_check) {
                Move temp_movelist[MAX_MOVES];
                struct PositionInfo position_info;
                do_move(position, &position_info, movelist[i]);
                const size_t temp_move_count = generate_legal_moves(position, temp_movelist);

                const Bitboard checkers = position->info->checkers;  // We still need this for double discovered checks.
                undo_move(position, movelist[i]);

                // Checks, else mates.
                if (temp_move_count != 0) {
                    if (direct_check && discovered_check) {
                        ++ext_perft->direct_discovered_checks;
                    } else if (direct_check) {
                        ++ext_perft->direct_checks;
                    } else if (!popcount64_greater_than_one(checkers)) {
                        // If we do not have a direct check, and there is only one checker, we must have a single
                        // discovered check, else a double discovered check.
                        ++ext_perft->single_discovered_checks;
                    } else {
                        ++ext_perft->double_discovered_checks;
                    }
                } else {
                    // Same as above but with mate variants.
                    if (direct_check && discovered_check) {
                        ++ext_perft->direct_discovered_mates;
                    } else if (direct_check) {
                        ++ext_perft->direct_mates;
                    } else if (!popcount64_greater_than_one(checkers)) {
                        // If we do not have a direct mater, and there is only one checker, we must have a single
                        // discovered mater, else a double discovered mater.
                        ++ext_perft->single_discovered_mates;
                    } else {
                        ++ext_perft->double_discovered_mates;
                    }
                }
            }
        }

        return move_count;
    }

    size_t nodes = 0;
    struct PositionInfo position_info;
    for (size_t i = 0; i < move_count; ++i) {
        do_move(position, &position_info, movelist[i]);
        nodes += extended_perft_nonzero_depth(position, depth - 1, ext_perft);
        undo_move(position, movelist[i]);
    }

    return nodes;
}

// Same as perft(), except it computes extra information and stores that in `ext_perft`.
static size_t extended_perft(struct Position* position, size_t depth, struct ExtendedPerft* ext_perft) {
    assert(position != nullptr);
    assert(ext_perft != nullptr);

    memset(ext_perft, 0, sizeof(*ext_perft));

    if (depth == 0)
        return 1;

    return extended_perft_nonzero_depth(position, depth, ext_perft);
}



#endif /* #ifndef WINDMOLEN_PERFT_H_ */
