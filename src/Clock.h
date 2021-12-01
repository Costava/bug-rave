#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Returns the number of nanoseconds since the C-implementation-defined epoch.
// If error, prints to `stderr` and calls `exit`.
// Requires C11 `timespec_get` in <time.h>.
uint64_t Clock_GetTimeNs(void);

// Clock_GetTimeNs but scaled to milliseconds and as type double.
double Clock_GetTimeMs(void);

#ifdef __cplusplus
}
#endif

#endif
