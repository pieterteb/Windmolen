#ifndef WINDMOLEN_ZOBRIST_H_
#define WINDMOLEN_ZOBRIST_H_

#include <assert.h>
#include <inttypes.h>

#include "types.h"
#include "util.h"



typedef uint64_t ZobristKey;


extern ZobristKey piece_zobrist_keys[PIECE_COUNT][SQUARE_COUNT];
extern ZobristKey castle_zobrist_keys[CASTLE_COUNT];
extern ZobristKey en_passant_zobrist_keys[FILE_COUNT];
extern ZobristKey side_to_move_zobrist_key;


void initialize_zobrist_keys();



#endif /* #ifndef WINDMOLEN_ZOBRIST_H_ */
