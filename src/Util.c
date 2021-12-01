#include "Util.h"

#include <stdio.h>
#include <stdlib.h>

time_t Util_TimeOrExit(time_t *const arg) {
    const time_t value = time(arg);

    if (value == (time_t)(-1)) {
        fprintf(stderr, "time(%p) errored\n", arg);
        exit(1);
    }

    return value;
}
