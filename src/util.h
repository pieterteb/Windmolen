#ifndef WINDMOLEN_UTIL_H_
#define WINDMOLEN_UTIL_H_


#include <assert.h>
#include <stdint.h>



#define INLINE inline __attribute__((always_inline))

#define IS_SAME_TYPE(T1, T2) _Generic((T1){0}, T2: true, default: false)


// Seeds the 64-bit pseudorandom number generator with `seed`.
void seed_rand64(uint64_t seed);

// Returns a pseudorandom 64-bit integer.
uint64_t rand64();

// Returns a pseudorandom 64-bit integer with on average only 1/8 of the bits set to 1.
uint64_t sparse_rand64();


// Returns the index of the least significant bit of a nonzero integer.
static INLINE int lsb64(uint64_t x) {
    assert(x != 0);

#ifdef __GNUC__
    return __builtin_ctzll(x);
#elifdef _MSC_VER
    unsigned long index;
    _BitScanForward64(&index, x);
    return (int)index;
#else
    // Fallback.
    // clang-format off
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
    // clang-format on
    return index64[((x & -x) * 0x03f79d71b4cb0a89ULL) >> 58];
#endif /* #ifdef __GNUC__ */
}

// Returns the number of 1-bits of `x`.
static INLINE int popcount64(uint64_t x) {
#ifdef __GNUC__
    return __builtin_popcountll(x);
#elifdef _MSC_VER
    return _mm_popcnt_u64(x);
#else
    // Fallback.
    x *= 0x0002000400080010ULL;
    x &= 0x1111111111111111ULL;
    x *= 0x1111111111111111ULL;
    x >>= 60;
    return (int)x;
#endif /* #ifdef __GNUC__ */
}

// Removes the least significant bit of `x` and returns its index.
static INLINE int pop_lsb64(uint64_t* x) {
    assert(*x != 0);

    int lsb_index = lsb64(*x);
    *x &= *x - 1;  // pop lsb.

    return lsb_index;
}


// Returns whether the number of 1-bits in `x` is greater than one.
static INLINE bool popcount64_greater_than_one(uint64_t x) {
    return (x & (x - 1)) != 0;
}



#endif /* #ifndef UTIL_H_ */
