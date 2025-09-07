#include <assert.h>
#include <stdint.h>

#include "util.h"



static uint64_t s;

void seed_rand64(uint64_t seed) {
    assert(seed);
    s = seed;
}

uint64_t rand64() {
    s ^= s >> 12;
    s ^= s << 25;
    s ^= s >> 27;
    return 2685821657736338717ull * s;
}

uint64_t sparse_rand64() {
    return rand64() & rand64() & rand64();
}
