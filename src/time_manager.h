#ifndef WINDMOLEN_TIME_MANAGER_H_
#define WINDMOLEN_TIME_MANAGER_H_


#include <stddef.h>
#include <stdint.h>

#include "piece.h"
#include "util.h"



#if defined(__linux__) || defined(__APPLE__)
#    include <sys/time.h>

static INLINE uint64_t get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return 1000000ULL * (uint64_t)tv.tv_sec + (uint64_t)tv.tv_usec;
}
#elifdef _WIN32
#    include <windows.h>

static INLINE uint64_t get_time_us() {
    static LARGE_INTEGER frequency;
    static bool frequency_initialized = 0;
    LARGE_INTEGER counter;

    if (!frequency_initialized) {
        QueryPerformanceFrequency(&frequency);
        frequency_initialized = true;
    }

    QueryPerformanceCounter(&counter);
    return ((1000000ULL * (uint64_t)counter.QuadPart) / (uint64_t)frequency.QuadPart);
}
#else
#    error "Unsupported platform."
#endif /* #if defined(__linux__) || defined(__APPLE__) */


// This structure contains any time related parameters. All time related parameters are stored in microseconds.
struct TimeManager {
    uint64_t white_time;
    uint64_t white_increment;
    uint64_t black_time;
    uint64_t black_increment;

    uint64_t move_time;
    size_t moves_to_go;

    uint64_t cutoff_time;

    uint64_t move_overhead;
};


// Sets all elements of `time_manager` except for `move_overhead` to 0.
void reset_time_manager(struct TimeManager* time_manager);

// Computes and updates the `cutoff_time` element in `time_manager` for `side_to_move`. This value is used to determine
// when to stop searching the current position.
void update_time_manager(struct TimeManager* time_manager, enum Color side_to_move);



#endif /* #ifndef WINDMOLEN_TIME_MANAGER_H_ */
