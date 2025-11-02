#ifndef WINDMOLEN_OPTIONS_H_
#define WINDMOLEN_OPTIONS_H_

#include <stddef.h>
#include <stdint.h>



#define HASH_OPTION_NAME  "Hash"
#define MIN_HASH_SIZE     ((uint64_t)1)
#define MAX_HASH_SIZE     ((uint64_t)1)
#define DEFAULT_HASH_SIZE ((uint64_t)1)

#define THREAD_OPTION_NAME   "Threads"
#define MIN_THREAD_COUNT     ((uint64_t)1)
#define MAX_THREAD_COUNT     ((uint64_t)1)
#define DEFAULT_THREAD_COUNT ((uint64_t)1)

#define MULTIPV_OPTION_NAME "MultiPV"
#define MIN_MULTIPV         ((uint64_t)1)
#define MAX_MULTIPV         ((uint64_t)1)
#define DEFAULT_MULTIPV     ((uint64_t)1)

struct Options {
    uint64_t hash_size;
    size_t thread_count;
    size_t multipv;
};



#endif /* #ifndef WINDMOLEN_OPTIONS_H_ */
