#ifndef WINDMOLEN_TIME_MANAGER_H_
#define WINDMOLEN_TIME_MANAGER_H_


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "types.h"

#if defined(__linux__) || defined(__APPLE__)
#    include <sys/time.h>

static inline uint64_t get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec;
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
    return (((uint64_t)counter.QuadPart * 1000000) / (uint64_t)frequency.QuadPart);
}
#else
#    error "Unsupported platform."
#endif /* #if defined(__linux__) || defined(__APPLE__) */



struct TimeManager {
    uint64_t white_time;
    uint64_t white_increment;
    uint64_t black_time;
    uint64_t black_increment;

    uint64_t white_cutoff_time;
    uint64_t black_cutoff_time;
    uint64_t cutoff_time;
};


void set_time_manager(struct TimeManager* time_manager, Color side_to_move);



#endif /* #ifndef WINDMOLEN_TIME_MANAGER_H_ */
