#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "bitboard.h"
#include "types.h"
#include "util.h"



#define MAX_TARGET_SQUARES  12U // The rook from one of the corner squares.
#define MAX_OCCUPANCIES     1U << MAX_TARGET_SQUARES // Maximum number of unique occupancies. Each target square can be occupied or not.


Bitboard sliding_attacks(PieceType piece_type, Bitboard square_bitboard, Bitboard occupancy) {
    assert(popcount64(square_bitboard) == 1 && (piece_type == PIECE_TYPE_BISHOP || piece_type == PIECE_TYPE_ROOK));

    Direction bishop_directions[4] = { DIRECTION_NORTHEAST, DIRECTION_SOUTHEAST, DIRECTION_SOUTHWEST, DIRECTION_NORTHWEST };
    Direction rook_directions[4] = { DIRECTION_NORTH, DIRECTION_EAST, DIRECTION_SOUTH, DIRECTION_WEST };
    Direction* directions = (piece_type == PIECE_TYPE_BISHOP) ? bishop_directions : rook_directions;

    Bitboard attacks = BITBOARD_EMPTY;

    for (size_t i = 0; i < 4; ++i) {
        Direction direction = directions[i];

        for (Bitboard bitboard = SHIFT_BITBOARD(square_bitboard, direction); bitboard; bitboard = SHIFT_BITBOARD(bitboard, direction)) {
            attacks |= bitboard;
            if (bitboard & occupancy) break;
        }
    }

    return attacks;
}


Bitboard find_magic(PieceType piece_type, Square square) {
    assert(piece_type == PIECE_TYPE_BISHOP || piece_type == PIECE_TYPE_ROOK);

    const Bitboard square_bitboard = SQUARE_BITBOARD(square);

    const Bitboard edges = ((FILE_A_BITBOARD | FILE_H_BITBOARD) & ~file_bitboard(file_from_square(square)))
                         | ((RANK_1_BITBOARD | RANK_8_BITBOARD) & ~rank_bitboard(rank_from_square(square)));
    const Bitboard mask = sliding_attacks(piece_type, square_bitboard, BITBOARD_EMPTY) & ~edges;

    const unsigned shift = 64U - (unsigned)popcount64(mask);

    Bitboard occupancy[MAX_OCCUPANCIES];
    Bitboard reference_attacks[MAX_OCCUPANCIES];

    size_t size = 0;
    Bitboard subset = 0;
    do {
        occupancy[size] = subset;
        reference_attacks[size] = sliding_attacks(piece_type, square_bitboard, subset);
        subset = (subset - mask) & mask;
        ++size;
    } while (subset);

    Bitboard attacks[MAX_OCCUPANCIES];
    size_t age[MAX_OCCUPANCIES] = { 0 };

    Bitboard magic;
    bool is_magical;
    size_t attempts = 1;
    do {
        magic = sparse_rand64();
        is_magical = true;

        for (size_t i = 0; i < size; ++i) {
            unsigned idx = (unsigned)((occupancy[i] * magic) >> shift);

            if (age[idx] != attempts) {
                age[idx] = attempts;
                attacks[idx] = reference_attacks[i];
            } else if (attacks[idx] != reference_attacks[i]) {
                is_magical = false;
                break;
            }
        }

        ++attempts;
    } while (!is_magical);

    return magic;
}


int main(void) {
    uint64_t seed = (uint64_t)time(NULL);
    seed_rand64(seed);
    printf("Seed used: %lu\n", seed);

    printf("\nBishop magics:\n");
    for (Square square = SQUARE_A1; square < SQUARE_COUNT; ++square)
        printf("%#018lx\n", find_magic(PIECE_TYPE_BISHOP, square));

    printf("\nRook magics:\n");
    for (Square square = SQUARE_A1; square < SQUARE_COUNT; ++square)
        printf("%#018lx\n", find_magic(PIECE_TYPE_ROOK, square));

    return 0;
}
