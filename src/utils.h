#ifndef UTILS_H
#define UTILS_H


#include <stdint.h>



void seed_rand64(uint64_t seed);
uint64_t rand64();


static inline int ctzll(unsigned long long x) {
#if defined(__GNUC__)
    return __builtin_ctzll(x);
#elif defined(_MSC_VER)
    unsigned long index;
    _BitScanForward(&index, x);
    return (int)index;
#else
    // Fallback.
    int i = 0;
    while ((x & 1) == 0) {
        x >>= 1;
        ++i;
    }
    return i
#endif /* defined(__GNUC__) */
}



#endif /* UTILS_H */