#ifndef WINDMOLEN_UCI_H_
#define WINDMOLEN_UCI_H_


#include <stddef.h>
#include <stdint.h>

#include "engine.h"
#include "move.h"
#include "score.h"



// Print `move` in UCI format to `stdout`.
void print_move(const Move move);

// Prints `best_move` in UCI format to `stdout`.
void uci_best_move(Move best_move);
void uci_long_info(size_t depth, size_t multipv, Score score, size_t nodes, uint64_t time, Move best_move);

// Run the main UCI loop.
void uci_loop(struct Engine* engine);



#endif /* #ifndef WINDMOLEN_UCI_H_ */
