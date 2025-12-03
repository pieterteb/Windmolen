#ifndef WINDMOLEN_PERFT_H_
#define WINDMOLEN_PERFT_H_


#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "move.h"
#include "move_generation.h"
#include "position.h"
#include "uci.h"
#include "util.h"



// Computes the number of leaf nodes in the chess search tree at nonzero `depth` in `position`.
static size_t perft_depth_nonzero(struct Position* position, const size_t depth) {
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
        nodes += perft_depth_nonzero(position, depth - 1);
        undo_move(position, movelist[i]);
    }

    return nodes;
}

// Computes the number of leaf nodes in the chess search tree at `depth` in `position`.
static INLINE size_t perft(struct Position* position, const size_t depth) {
    assert(position != nullptr);

    if (depth == 0)
        return 1;

    return perft_depth_nonzero(position, depth);
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


// Struct to keep data of extended perft. To clarify, a move can fall under multiple categories. For example, an en
// passant move is also a capture, and a checkmate is also a check, and can potentially be a discovery check etc.
struct ExtendedPerft {
    size_t capture;
    size_t en_passant;
    size_t castle;
    size_t promotion;
    size_t check;
    size_t discovery_check;
    size_t double_check;
    size_t checkmate;
    size_t double_checkmate;
};

// Same as perft_depth_nonzero(), except it computes extra information and stores that in `ext_perft`.
static size_t extended_perft_depth_nonzero(struct Position* position, size_t depth, struct ExtendedPerft* ext_perft) {
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
                ++ext_perft->capture;

            // These three cases are mutually exclusive.
            if (move_type == MOVE_TYPE_EN_PASSANT) {
                // An en passant move is always a capture.
                ++ext_perft->capture;
                ++ext_perft->en_passant;
            } else if (move_type == MOVE_TYPE_CASTLE) {
                ++ext_perft->castle;
            } else if (move_type == MOVE_TYPE_PROMOTION) {
                ++ext_perft->promotion;
            }


            const bool direct_check    = is_direct_check(position, movelist[i]);
            const bool discovery_check = is_discovery_check(position, movelist[i]);

            if (direct_check | discovery_check) {
                ++ext_perft->check;

                if (discovery_check)
                    ++ext_perft->discovery_check;

                Move temp_movelist[MAX_MOVES];
                const size_t temp_move_count = generate_legal_moves(position, temp_movelist);

                if (temp_move_count == 0)
                    ++ext_perft->checkmate;

                // A double check occurs if and only if we have a direct check and a discovery check.
                if (direct_check && discovery_check) {
                    ++ext_perft->double_check;

                    if (move_count == 0)
                        ++ext_perft->double_checkmate;
                }
            }
        }

        return move_count;
    }

    size_t nodes = 0;
    struct PositionInfo position_info;
    for (size_t i = 0; i < move_count; ++i) {
        do_move(position, &position_info, movelist[i]);
        nodes += extended_perft_depth_nonzero(position, depth - 1, ext_perft);
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

    return extended_perft_depth_nonzero(position, depth, ext_perft);
}



#endif /* #ifndef WINDMOLEN_PERFT_H_ */
