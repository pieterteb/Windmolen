#ifndef WINDMOLEN_TIME_MANAGER_H_
#define WINDMOLEN_TIME_MANAGER_H_


#include <stddef.h>
#include <stdint.h>

#include "types.h"

#if defined(__linux__) || defined(__APPLE__)
#    include <sys/time.h>

static inline uint64_t get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 1000000ULL * (uint64_t)tv.tv_sec + (uint64_t)tv.tv_usec;
}
#elif defined(_WIN32)
#    include <windows.h>

static inline uint64_t get_time_us() {
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



struct TimeManager {
    uint64_t white_time;
    uint64_t white_increment;
    uint64_t black_time;
    uint64_t black_increment;
    uint64_t move_time;
    size_t moves_to_go;

    uint64_t cutoff_time;
};


void reset_time_manager(struct TimeManager* time_manager);
void update_time_manager(struct TimeManager* time_manager, enum Color side_to_move);



#endif /* #ifndef WINDMOLEN_TIME_MANAGER_H_ */
