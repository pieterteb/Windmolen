#include <assert.h>
#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "bitboard.h"
#include "types.h"
#include "util.h"


#define sliding_attacks(piece_type, square_bitboard, occupancy) (                            \
      (piece_type) == PIECE_TYPE_BISHOP ? sliding_attacks_bishop(square_bitboard, occupancy) \
    : (piece_type) == PIECE_TYPE_ROOK   ? sliding_attacks_rook(square_bitboard, occupancy)   \
    : BITBOARD_EMPTY                                                                         \
)

Bitboard sliding_attacks_bishop(Bitboard square_bitboard, Bitboard occupancy) {
    assert(popcount64(square_bitboard) == 1);
    Bitboard attacks = BITBOARD_EMPTY;

    // Northeast direction.
    for (Bitboard bitboard = SHIFT_BITBOARD(square_bitboard, DIRECTION_NORTHEAST); bitboard; bitboard = SHIFT_BITBOARD(bitboard, DIRECTION_NORTHEAST)) {
        attacks |= bitboard;
        if (bitboard & occupancy) break;
    }
    
    // Southeast direction.
    for (Bitboard bitboard = SHIFT_BITBOARD(square_bitboard, DIRECTION_SOUTHEAST); bitboard; bitboard = SHIFT_BITBOARD(bitboard, DIRECTION_SOUTHEAST)) {
        attacks |= bitboard;
        if (bitboard & occupancy) break;
    }
    
    // Southwest direction.
    for (Bitboard bitboard = SHIFT_BITBOARD(square_bitboard, DIRECTION_SOUTHWEST); bitboard; bitboard = SHIFT_BITBOARD(bitboard, DIRECTION_SOUTHWEST)) {
        attacks |= bitboard;
        if (bitboard & occupancy) break;
    }

    // Northwest direction.
    for (Bitboard bitboard = SHIFT_BITBOARD(square_bitboard, DIRECTION_NORTHWEST); bitboard; bitboard = SHIFT_BITBOARD(bitboard, DIRECTION_NORTHWEST)) {
        attacks |= bitboard;
        if (bitboard & occupancy) break;
    }

    return attacks;
}

Bitboard sliding_attacks_rook(Bitboard square_bitboard, Bitboard occupancy) {
    assert(popcount64(square_bitboard) == 1);
    Bitboard attacks = BITBOARD_EMPTY;

    // Northeast direction.
    for (Bitboard bitboard = SHIFT_BITBOARD(square_bitboard, DIRECTION_NORTH); bitboard; bitboard = SHIFT_BITBOARD(bitboard, DIRECTION_NORTH)) {
        attacks |= bitboard;
        if (bitboard & occupancy) break;
    }
    
    // Southeast direction.
    for (Bitboard bitboard = SHIFT_BITBOARD(square_bitboard, DIRECTION_EAST); bitboard; bitboard = SHIFT_BITBOARD(bitboard, DIRECTION_EAST)) {
        attacks |= bitboard;
        if (bitboard & occupancy) break;
    }
    
    // Southwest direction.
    for (Bitboard bitboard = SHIFT_BITBOARD(square_bitboard, DIRECTION_SOUTH); bitboard; bitboard = SHIFT_BITBOARD(bitboard, DIRECTION_SOUTH)) {
        attacks |= bitboard;
        if (bitboard & occupancy) break;
    }

    // Northwest direction.
    for (Bitboard bitboard = SHIFT_BITBOARD(square_bitboard, DIRECTION_WEST); bitboard; bitboard = SHIFT_BITBOARD(bitboard, DIRECTION_WEST)) {
        attacks |= bitboard;
        if (bitboard & occupancy) break;
    }

    return attacks;
}


#define MAX_TARGET_SQUARES  12U // The rook from one of the corner squares.
#define MAX_OCCUPANCIES     1U << MAX_TARGET_SQUARES // Maximum number of unique occupancies. Each target square can be occupied or not.

void find_magics(PieceType piece_type, Square square) {
    assert(piece_type == PIECE_TYPE_BISHOP || piece_type == PIECE_TYPE_ROOK);

    const Bitboard square_bitboard = SQUARE_BITBOARD(square);

    const Bitboard edges = ((FILE_A_BITBOARD | FILE_H_BITBOARD) & ~file_bitboard(file_from_square(square)))
                         | ((RANK_1_BITBOARD | RANK_8_BITBOARD) & ~rank_bitboard(rank_from_square(square)));
    const Bitboard mask = sliding_attacks(piece_type, square_bitboard, BITBOARD_EMPTY) & ~edges;

    const unsigned relevant_bits = (unsigned)popcount64(mask);
    const unsigned shift = 64U - relevant_bits;
    const unsigned occupancy_count = 1U << relevant_bits;

    Bitboard occupancy[MAX_OCCUPANCIES] __attribute__((aligned(64)));
    Bitboard reference_attacks[MAX_OCCUPANCIES] __attribute__((aligned(64)));

    size_t size = 0;
    for (Bitboard subset = 0; ; subset = (subset - mask) & mask) {
        occupancy[size] = subset;
        reference_attacks[size] = sliding_attacks(piece_type, square_bitboard, subset);
        if (++size == occupancy_count) break;
    }

    Bitboard attacks[MAX_OCCUPANCIES] __attribute__((aligned(64)));
    size_t epoch[MAX_OCCUPANCIES] __attribute__((aligned(64)));
    for (size_t i = 0; i < MAX_OCCUPANCIES; ++i) epoch[i] = 0;

    Bitboard magic;
    unsigned min_max_idx = UINT32_MAX;
    size_t attempts = 1;
    while (1) {
        magic = sparse_rand64();
        unsigned max_idx = 0;
        bool is_magical = true;

        size_t i = 0;
        for (; i + 8 <= size; i += 8) {
            __m512i occupancy_vec = _mm512_load_si512(&occupancy[i]);
            __m512i magic_vec = _mm512_set1_epi64((long long)magic);
            __m512i product = _mm512_mullo_epi64(occupancy_vec, magic_vec);
            __m512i idx_vec = _mm512_srli_epi64(product, shift);

            uint64_t idx_array[8] __attribute__((aligned(64)));
            _mm512_store_si512((__m512i*)idx_array, idx_vec);

            for (size_t j = 0; j < 8; ++j) {
                unsigned idx = (unsigned)idx_array[j];

                if (idx > min_max_idx) {
                    is_magical = false;
                    break;
                }

                if (epoch[idx] != attempts) {
                    epoch[idx] = attempts;
                    attacks[idx] = reference_attacks[i + j];
                } else if (attacks[idx] != reference_attacks[i + j]) {
                    is_magical = false;
                    break;
                }

                if (idx > max_idx) max_idx = idx;
            }

            if (!is_magical) break;
        }

        for (; i < size && is_magical; ++i) {
            unsigned idx = (unsigned)((occupancy[i] * magic) >> shift);
            if (idx > min_max_idx) {
                is_magical = false;
                break;
            }

            if (epoch[idx] != attempts) {
                epoch[idx] = attempts;
                attacks[idx] = reference_attacks[i];
            } else if (attacks[idx] != reference_attacks[i]) {
                is_magical = false;
                break;
            }

            if (idx > max_idx) max_idx = idx;
        }

        // for (size_t i = 0; i < size; ++i) {
        //     unsigned idx = (unsigned)((occupancy[i] * magic) >> shift);
        //     if (idx > min_max_idx) {
        //         is_magical = false;
        //         break;
        //     }

        //     if (epoch[idx] != attempts) {
        //         epoch[idx] = attempts;
        //         attacks[idx] = reference_attacks[i];
        //     } else if (attacks[idx] != reference_attacks[i]) {
        //         is_magical = false;
        //         break;
        //     }

        //     if (idx > max_idx) max_idx = idx;
        // }

        if (is_magical && max_idx < min_max_idx) {
            min_max_idx = max_idx;
            printf("%#018lx with max idx %u after %zu attempts.\n", magic, min_max_idx, attempts);
        }

        ++attempts;
    }
}


int main(int argc, char* argv[]) {
    uint64_t seed = (uint64_t)time(NULL);
    seed_rand64(seed);
    printf("Seed used: %lu\n", seed);
    // seed_rand64(1);

    for (int i = 1; i < argc; ++i)
        printf("%s ", argv[0]);
    printf("\n");

    find_magics(PIECE_TYPE_BISHOP, SQUARE_A1);

    return 0;
}
