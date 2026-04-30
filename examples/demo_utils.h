#ifndef DEMO_UTILS_H
#define DEMO_UTILS_H

#include <time.h>

#include "vtkit/vtkit.h"

#if defined(__GNUC__) || defined(__clang__)
#define DEMO_UNUSED __attribute__((unused))
#else
#define DEMO_UNUSED
#endif

/* Monotonic clock in microseconds. */
static DEMO_UNUSED long long demo_now_usec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((long long) ts.tv_sec * 1000000LL) + (long long) (ts.tv_nsec / 1000L);
}

/* Block until a key is available (5ms poll), then return it. */
static DEMO_UNUSED int demo_wait_key(void) {
    struct timespec ts;
    int ch;

    ts.tv_sec = 0;
    ts.tv_nsec = 5000000; /* 5ms */

    while ((ch = vtk_getch()) == -1) {
        nanosleep(&ts, NULL);
    }
    return ch;
}

#endif
