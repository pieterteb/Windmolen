#ifndef UTILS_H
#define UTILS_H



inline int ctzll(unsigned long long x) {
#if defined(__GNUC__) || defined(__clang__)
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
#endif /* defined(__GNUC__) || defined(__clang__) */
}



#endif /* UTILS_H */