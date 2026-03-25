#ifndef WINDMOLEN_TRANSPOSITION_TABLE_H_
#define WINDMOLEN_TRANSPOSITION_TABLE_H_


#include <stddef.h>
#include <stdint.h>

#include "move.h"
#include "score.h"
#include "zobrist.h"



enum TTValueType {
    TT_VALUE_EXACT,
    TT_VALUE_LOWERBOUND,
    TT_VALUE_UPPERBOUND
};

struct TTEntry {
    ZobristKey key;
    size_t depth;
    Value value;
    uint8_t flag;  // Exact, lowerbound, upperbound.
    Move best_move;
};


struct TTEntry* tt_init(struct TTEntry* tt, size_t* tt_size, const size_t mb_size);
void tt_reset(struct TTEntry* tt, const size_t tt_size);
void tt_destroy(struct TTEntry* transposition_table);

int tt_probe(const struct TTEntry* tt, const size_t tt_size, const ZobristKey key, const size_t depth,
             const Value alpha, const Value beta, Value* value, Move* best_move);
void tt_store(struct TTEntry* tt, const size_t tt_size, const ZobristKey key, const size_t depth, const Value value,
              enum TTValueType flag, const Move best_move);



#endif  // #ifndef WINDMOLEN_TRANSPOSITION_TABLE_H_
