#ifndef WINDMOLEN_UCI_H_
#define WINDMOLEN_UCI_H_


#include <stdbool.h>
#include <stdint.h>

#include "engine.h"
#include "move_generation.h"
#include "position.h"



/* Run the main uci loop. */
void print_move(FILE* stream, Move move);

void uci_loop(struct Engine* engine);


void uci_best_move(Move best_move);


void uci_long_info(size_t depth, size_t multipv, Score score, size_t nodes, uint64_t time, Move best_move);



#endif /* #ifndef WINDMOLEN_UCI_H_ */
