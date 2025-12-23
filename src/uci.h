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
void uci_best_move(const Move best_move);
void uci_long_info(const size_t depth, const size_t multipv, Score score, const size_t nodes, const uint64_t time,
                   const Move* principal_variation, const size_t principal_variation_length);

// Run the main UCI loop.
void uci_loop(struct Engine* engine);



#endif /* #ifndef WINDMOLEN_UCI_H_ */
