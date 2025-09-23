#ifndef UTIL_H
#define UTIL_H


#include <assert.h>
#include <stdint.h>



void seed_rand64(uint64_t seed);
uint64_t rand64();
uint64_t sparse_rand64();


static inline int ctz64(unsigned long long x) {
    assert(x != 0);

#if defined(__GNUC__)
    return __builtin_ctzll(x);
#elif defined(_MSC_VER)
    unsigned long index;
    _BitScanForward64(&index, x);
    return (int)index;
#else
    // Fallback.
    static const int index64[64] = {
         0,  1, 48,  2, 57, 49, 28,  3,
        61, 58, 50, 42, 38, 29, 17,  4,
        62, 55, 59, 36, 53, 51, 43, 22,
        45, 39, 33, 30, 24, 18, 12,  5,
        63, 47, 56, 27, 60, 41, 37, 16,
        54, 35, 52, 21, 44, 32, 23, 11,
        46, 26, 40, 15, 34, 20, 31, 10,
        25, 14, 19,  9, 13,  8,  7,  6
    };
    return index64[((x & -x) * 0x03f79d71b4cb0a89ULL) >> 58];
#endif /* defined(__GNUC__) */
}

static inline int popcount64(unsigned long long x) {
#if defined(__GNUC__)
    return __builtin_popcountll(x);
#elif defined(_MSC_VER)
    return _mm_popcnt_u64(x);
#else
    // Fallback.
    x *= 0x0002000400080010ULL;
    x &= 0x1111111111111111ULL;
    x *= 0x1111111111111111ULL;
    x >>= 60;
    return (int)x;

    // x = x - ((x >> 1) & 0x55555555);
    // x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    // x = (x + (x >> 4)) & 0x0F0F0F0F;
    // x = x + (x >> 8);
    // x = x + (x >> 16);
    // return (int)(x & 0x3F);
#endif /* defined(__GNUC__) */
}

static inline int pop_lsb64(unsigned long long* x) {
    assert(*x != 0);

    int lsb_index = ctz64(*x);
    *x &= *x - 1; // pop lsb.

    return lsb_index;
}



#endif /* UTIL_H */