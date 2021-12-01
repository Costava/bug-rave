#ifndef MEM_H
#define MEM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// These functions call `malloc` or `realloc` accordingly.
// If error, print to `stderr` and exit
void *Mem_Alloc(size_t size);
void *Mem_Realloc(void *ptr, size_t newSize);

#ifdef __cplusplus
}
#endif

#endif
