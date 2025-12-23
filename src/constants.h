#ifndef WINDMOLEN_CONSTANTS_H_
#define WINDMOLEN_CONSTANTS_H_


#include <stddef.h>



// An upperbound for the maximum number of pseudolegal moves in a chess position.
static constexpr size_t MAX_MOVES        = 256;
static constexpr size_t MAX_SEARCH_DEPTH = 128;

// According to official FIDE rules, after 50 consecutive reversible moves have been played, both players are allowed to
// claim a draw, but are not required to. Both players may decide to play on, resulting in legal chess positions in
// which more than 50 consecutive reversible moves have been played. However, after 75 consecutive reversible moves, the
// game ends in a draw, unless the last move was a checkmate of course.
static constexpr size_t HALFMOVE_CLOCK_LIMIT = 150;



#endif /* #ifndef WINDMOLEN_CONSTANTS_H_ */
