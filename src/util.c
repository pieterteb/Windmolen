#include "util.h"

#include <assert.h>
#include <stdint.h>



/* This file currently only contains pseudorandom number generator related functions. Once zobrist keys are statically
 * initialized, these functions will become obsolete. */


static uint64_t s;

void seed_rand64(uint64_t seed) {
    assert(seed != 0);

    s = seed;
}

uint64_t rand64() {
    // Implementation of a xorshift* generator, as suggested by Marsaglia
    // (https://en.wikipedia.org/wiki/Xorshift#xorshift*).
    assert(s != 0);

    s ^= s >> 12;
    s ^= s << 25;
    s ^= s >> 27;
    return 0x2545f4914f6cdd1dull * s;
}

uint64_t sparse_rand64() {
    // By bitwise and-ing three pseudorandom integers, we obtain a pseudorandom integer with relatively little bits set
    // to 1 (1/8th).

    return rand64() & rand64() & rand64();
}
