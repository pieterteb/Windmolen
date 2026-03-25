#include "transposition_table.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "move.h"
#include "score.h"
#include "zobrist.h"



struct TTEntry* tt_init(struct TTEntry* tt, size_t* tt_size, const size_t mb_size) {
    const size_t bytes       = mb_size * 1024 * 1024;
    const size_t entry_count = bytes / sizeof(struct TTEntry);

    size_t new_size = 1;
    while ((new_size << 1) <= entry_count)
        new_size <<= 1;

    if (tt)
        free(tt);

    *tt_size = new_size;
    return calloc(new_size, sizeof(struct TTEntry));
}

void tt_reset(struct TTEntry* tt, const size_t tt_size) {
    memset(tt, 0, tt_size * sizeof(struct TTEntry));
}

void tt_destroy(struct TTEntry* transposition_table) {
    free(transposition_table);
}

int tt_probe(const struct TTEntry* tt, const size_t tt_size, const ZobristKey key, const size_t depth,
             const Value alpha, const Value beta, Value* value, Move* best_move) {
    const struct TTEntry* entry = &tt[key & (tt_size - 1)];

    if (entry->key == key) {
        if (entry->depth >= depth) {
            if (entry->flag == TT_VALUE_EXACT) {
                *value     = entry->value;
                *best_move = entry->best_move;
                return 1;
            } else if (entry->flag == TT_VALUE_LOWERBOUND && entry->value >= beta) {
                *value     = entry->value;
                *best_move = entry->best_move;
                return 1;
            } else if (entry->flag == TT_VALUE_UPPERBOUND && entry->value <= alpha) {
                *value     = entry->value;
                *best_move = entry->best_move;
                return 1;
            }
        }
    }

    return 0;
}

void tt_store(struct TTEntry* tt, const size_t tt_size, const ZobristKey key, const size_t depth, const Value value,
              enum TTValueType flag, const Move best_move) {
    struct TTEntry* entry = &tt[key & (tt_size - 1)];

    if (entry->key != key || depth >= entry->depth) {
        entry->key       = key;
        entry->depth     = depth;
        entry->value     = value;
        entry->flag      = (uint8_t)flag;
        entry->best_move = best_move;
    }
}
