#ifndef UTIL_H
#define UTIL_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// time but, if error, print to stderr and exit.
time_t Util_TimeOrExit(time_t *const arg);

#ifdef __cplusplus
}
#endif

#endif // UTIL_H
