#include "Clock.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

uint64_t Clock_GetTimeNs(void) {
    struct timespec ts;

    if (timespec_get(&ts, TIME_UTC) == 0) {
        fprintf(stderr, "%s: timespec_get error\n", __func__);
        exit(EXIT_FAILURE);
    }

    return ((uint64_t)ts.tv_sec) * 1000000000llu + ((uint64_t)ts.tv_nsec);
}

double Clock_GetTimeMs(void) {
    return ((double)Clock_GetTimeNs()) / 1000000.0;
}
