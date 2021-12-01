/*
File: vec.h
Author: Costava
License: BSD 2-Clause License (see end of file)

A vec is a struct to manage the state of a dynamic array.
The array buffer is heap-allocated and self-resizing (grows when necessary).
For a given vec, all elements have the same type.
Buffer size can grow linearly or exponentially (set when initializing struct).
All functions will print to stderr and exit if a memory allocation fails.

Always call _init on a vec before using it.
Call _deinit to clean up and free the internals of a vec.

If you want to use vec with a certain type,
  you will need to generate the code.
There are two ways to generate vec code for a given type:
1. Use macro VEC_GENERATE_HEADER_CODE in the type's header file.
   Use macro VEC_GENERATE_IMPLEMENTATION_CODE in the type's .c file.
   Explanation of macro parameters:
   - type e.g. char, struct foo, unsigned char
   - suffix is what will follow vec in the function names
       e.g. vecchar_push_back
            when char is given as the suffix in the macro call.
       e.g. vecstructfoo_push_back
       e.g. vecfoo_push_back
       e.g. vecunsignedchar_push_back
       e.g. vecuchar_push_back

2. Use macro VEC_GENERATE_FOR_TYPE in the main file
     or an implementation file (when forward declarations are not needed).
   For explanation of macro parameters, see 1.
*/

#ifndef VEC_H
#define VEC_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

// Multiply the size of the buffer by the grow_val
#define VEC_GROW_MODE_MULTIPLY 1
// Add grow_val number of spots to the buffer
#define VEC_GROW_MODE_ADD 2

// Max permitted value of capacity member of struct.
// Leave 1 space so that a normal for loop can iterate the buf.
#define VEC_MAX_CAPACITY (SIZE_MAX - 1)

////////////////////////////////////////////////////////////////////////////////

// Generates header code for the given type.
// suffix is used in the function names.
// e.g. vecint_pop_back if suffix is int
// e.g. vecunsignedchar_push_back if suffix is unsignedchar
#define VEC_GENERATE_HEADER_CODE(type, suffix)                                 \
    typedef struct vec##suffix {                                               \
        /* These members are expected to be touched in order to                \
         * iterate over all items in vec                                       \
         */                                                                    \
        type *buf;                                                             \
        size_t length; /* Number of elements in buf */                         \
                                                                               \
        /* Do not touch the below members outside of vec functions */          \
                                                                               \
        /* The number of spots allocated for the buf */                        \
        size_t capacity;                                                       \
        /* The way that the buffer size should grow when space is needed */    \
        /* Use the VEC_GROW_MODE_... constants to specify a value */           \
        uint8_t grow_mode;                                                     \
        /* Usage depends on the grow_mode value */                             \
        size_t grow_val;                                                       \
    } vec##suffix;                                                             \
                                                                               \
    /* Clean up and free any internals of vec.                                 \
     */                                                                        \
    void vec##suffix##_deinit(vec##suffix *const vec);                         \
                                                                               \
    /* Do not use a vec struct before calling _init on it.                     \
     */                                                                        \
    void vec##suffix##_init(vec##suffix *const vec, const size_t capacity,     \
        const uint8_t grow_mode, const size_t grow_val);                       \
                                                                               \
    /* Insert val into vec at given index.                                     \
     * Shift values further into buf to make space if necessary.               \
     * Print to stderr and exit(1) if error.                                   \
     * If vec already has VEC_MAX_CAPACITY members, prints to stderr and exits.\
     */                                                                        \
    void vec##suffix##_insert_at_shift(                                        \
        vec##suffix *const vec, const type val, const size_t index);           \
                                                                               \
    /* Insert val into vec at given index.                                     \
     * Swap value at index to end of buf if necessary.                         \
     * Print to stderr and exit(1) if error.                                   \
     * If vec already has VEC_MAX_CAPACITY members, prints to stderr and exits.\
     */                                                                        \
    void vec##suffix##_insert_at_swap(                                         \
        vec##suffix *const vec, const type val, const size_t index);           \
                                                                               \
    /* Remove and return the value at the end of the vec.                      \
     * Print to stderr and exit if vec is empty.                               \
     */                                                                        \
    type vec##suffix##_pop_back(vec##suffix *const vec);                       \
                                                                               \
    /* Append val to end of vec.                                               \
     * If vec already has VEC_MAX_CAPACITY members, prints to stderr and exits.\
     */                                                                        \
    void vec##suffix##_push_back(vec##suffix *const vec, type val);            \
                                                                               \
    /* Remove and return the value at the given index.                         \
     * Shift elements over to fill hole.                                       \
     * If error, print to stderr and exit(1)                                   \
     */                                                                        \
    type vec##suffix##_remove_at_shift(                                        \
        vec##suffix *const vec, const size_t index);                           \
                                                                               \
    /* Remove and return the value at the given index.                         \
     * Move the last element in buf into the space created by the removal.     \
     * If error, print to stderr and exit(1)                                   \
     */                                                                        \
    type vec##suffix##_remove_at_swap(                                         \
        vec##suffix *const vec, const size_t index);                           \
                                                                               \
    /* Shrink buffer to the size of its current contents.                      \
     */                                                                        \
    void vec##suffix##_shrink_to_fit(vec##suffix *const vec);


// Generates implementation code for the given type.
// See above for explanation of suffix.
#define VEC_GENERATE_IMPLEMENTATION_CODE(type, suffix)                         \
    /* Reallocate the buf for vec based on current capacity value.             \
     * Print to stderr and exit if the bytes to allocate overflows size_t.     \
     * You should never need to call this (other vec functions will).          \
     */                                                                        \
    static void vec##suffix##_reallocate_buf(vec##suffix *const vec)           \
    {                                                                          \
        const size_t new_size = sizeof(type) * vec->capacity;                  \
                                                                               \
        if ((new_size / sizeof(type)) != vec->capacity)                        \
        {                                                                      \
            fprintf(stderr,                                                    \
                "%s: Number of bytes to allocate overflows size_t. "           \
                "[capacity: %zu] [sizeof(type): %zu]\n", __func__,             \
                vec->capacity, sizeof(type));                                  \
                                                                               \
            exit(1);                                                           \
        }                                                                      \
                                                                               \
        if (new_size > 0) {                                                    \
            vec->buf = realloc(vec->buf, new_size);                            \
                                                                               \
            if (vec->buf == NULL) {                                            \
                fprintf(stderr, "%s: Failed to realloc to %zu bytes\n",        \
                    __func__, new_size);                                       \
                                                                               \
                exit(1);                                                       \
            }                                                                  \
        }                                                                      \
        else {                                                                 \
            free(vec->buf);                                                    \
            vec->buf = NULL;                                                   \
        }                                                                      \
    }                                                                          \
                                                                               \
    /* Ensure vec has space for at least 1 more value.                         \
     * Else print to stderr and exit if fail to make space.                    \
     * You should never need to call this (other vec functions will).          \
     */                                                                        \
    static void vec##suffix##_ensure_space(vec##suffix *const vec)             \
    {                                                                          \
        if (vec->length == vec->capacity)                                      \
        {                                                                      \
            if (vec->capacity == VEC_MAX_CAPACITY) {                           \
                fprintf(stderr, "%s: Cannot add to full, max-sized vec.\n",    \
                    __func__);                                                 \
                                                                               \
                exit(1);                                                       \
            }                                                                  \
            else if (vec->capacity > VEC_MAX_CAPACITY) {                       \
                fprintf(stderr, "%s: vec is in invalid state. capacity: %zu "  \
                    "VEC_MAX_CAPACITY: %zu\n", __func__,                       \
                    vec->capacity, VEC_MAX_CAPACITY);                          \
                                                                               \
                exit(1);                                                       \
            }                                                                  \
                                                                               \
            switch (vec->grow_mode) {                                          \
                case VEC_GROW_MODE_MULTIPLY:                                   \
                {                                                              \
                    size_t new_capacity = vec->capacity * vec->grow_val;       \
                    if ((new_capacity / vec->capacity) != vec->grow_val)       \
                    {                                                          \
                        /* Multiplying would overflow capacity so we have to   \
                         * settle for the maximum value.                       \
                         */                                                    \
                        new_capacity = VEC_MAX_CAPACITY;                       \
                    }                                                          \
                                                                               \
                    vec->capacity = new_capacity;                              \
                    vec##suffix##_reallocate_buf(vec);                         \
                } break;                                                       \
                case VEC_GROW_MODE_ADD:                                        \
                {                                                              \
                    if (vec->capacity <= VEC_MAX_CAPACITY - vec->grow_val)     \
                    {                                                          \
                        vec->capacity += vec->grow_val;                        \
                    }                                                          \
                    else                                                       \
                    {                                                          \
                        vec->capacity = VEC_MAX_CAPACITY;                      \
                    }                                                          \
                                                                               \
                    vec##suffix##_reallocate_buf(vec);                         \
                } break;                                                       \
                default:                                                       \
                {                                                              \
                    fprintf(stderr, "%s: Unknown grow_mode value: %d\n",       \
                        __func__, vec->grow_mode);                             \
                                                                               \
                    exit(1);                                                   \
                } break;                                                       \
            }                                                                  \
        }                                                                      \
        else if (vec->length > vec->capacity) {                                \
            fprintf(stderr, "%s: vec is in invalid state. length: %zu "        \
                "capacity: %zu\n", __func__, vec->length, vec->capacity);      \
                                                                               \
            exit(1);                                                           \
        }                                                                      \
    }                                                                          \
                                                                               \
    void vec##suffix##_deinit(vec##suffix *const vec)                          \
    {                                                                          \
        free(vec->buf);                                                        \
        vec->buf    = NULL;                                                    \
        vec->length    = 0;                                                    \
        vec->capacity = 0;                                                     \
    }                                                                          \
                                                                               \
    void vec##suffix##_init(vec##suffix *const vec, const size_t capacity,     \
        const uint8_t grow_mode, const size_t grow_val)                        \
    {                                                                          \
        /* calloc because able to separately specify num and size. */          \
        /* i.e. calloc will have to worry about possible overflow. */          \
        if (capacity > 0) {                                                    \
            vec->buf = calloc(capacity, sizeof(type));                         \
                                                                               \
            if (vec->buf == NULL) {                                            \
                fprintf(stderr, "%s: Failed to calloc %zu of size %zu\n",      \
                    __func__, capacity, sizeof(type));                         \
                                                                               \
                exit(1);                                                       \
            }                                                                  \
        }                                                                      \
        else {                                                                 \
            vec->buf = NULL;                                                   \
        }                                                                      \
                                                                               \
        vec->length    = 0;                                                    \
        vec->capacity  = capacity;                                             \
        vec->grow_mode = grow_mode;                                            \
                                                                               \
        if (grow_val == 0) {                                                   \
            fprintf(stderr, "%s: grow_val must be non-zero.\n", __func__);     \
            exit(1);                                                           \
        }                                                                      \
        else if (grow_mode == VEC_GROW_MODE_MULTIPLY && grow_val == 1) {       \
            fprintf(stderr, "%s: grow_val must be >=2 for "                    \
                "VEC_GROW_MODE_MULTIPLY.\n", __func__);                        \
                                                                               \
            exit(1);                                                           \
        }                                                                      \
                                                                               \
        vec->grow_val  = grow_val;                                             \
    }                                                                          \
                                                                               \
    void vec##suffix##_insert_at_shift(                                        \
        vec##suffix *const vec, const type val, const size_t index)            \
    {                                                                          \
        if (index > vec->length) {                                             \
            fprintf(stderr, "%s: Cannot insert into vec of length %zu "        \
                "at index %zu\n", __func__, vec->length, index);               \
                                                                               \
            exit(1);                                                           \
        }                                                                      \
        else if (index == vec->length) {                                       \
            vec##suffix##_ensure_space(vec);                                   \
            vec->buf[index] = val;                                             \
            vec->length += 1;                                                  \
        }                                                                      \
        else {                                                                 \
            vec##suffix##_ensure_space(vec);                                   \
            /* Stop at 1 above index because index may be 0 */                 \
            for (size_t i = vec->length; i > index + 1; i -= 1) {              \
                vec->buf[i] = vec->buf[i - 1];                                 \
            }                                                                  \
            vec->buf[index + 1] = vec->buf[index];                             \
            vec->buf[index] = val;                                             \
            vec->length += 1;                                                  \
        }                                                                      \
    }                                                                          \
                                                                               \
    void vec##suffix##_insert_at_swap(                                         \
        vec##suffix *const vec, const type val, const size_t index)            \
    {                                                                          \
        if (index > vec->length) {                                             \
            fprintf(stderr, "%s: Cannot insert into vec of length %zu "        \
                "at index %zu\n", __func__, vec->length, index);               \
                                                                               \
            exit(1);                                                           \
        }                                                                      \
        else if (index == vec->length) {                                       \
            vec##suffix##_ensure_space(vec);                                   \
            vec->buf[index] = val;                                             \
            vec->length += 1;                                                  \
        }                                                                      \
        else {                                                                 \
            vec##suffix##_ensure_space(vec);                                   \
            vec->buf[vec->length] = vec->buf[index];                           \
            vec->buf[index] = val;                                             \
            vec->length += 1;                                                  \
        }                                                                      \
    }                                                                          \
                                                                               \
    type vec##suffix##_pop_back(vec##suffix *const vec)                        \
    {                                                                          \
        if (vec->length == 0) {                                                \
            fprintf(stderr, "%s: Cannot pop from empty vec.\n", __func__);     \
            exit(1);                                                           \
        }                                                                      \
                                                                               \
        vec->length -= 1;                                                      \
        return vec->buf[vec->length];                                          \
    }                                                                          \
                                                                               \
    void vec##suffix##_push_back(vec##suffix *const vec, type val)             \
    {                                                                          \
        vec##suffix##_ensure_space(vec);                                       \
        vec->buf[vec->length] = val;                                           \
        vec->length += 1;                                                      \
    }                                                                          \
                                                                               \
    type vec##suffix##_remove_at_shift(                                        \
        vec##suffix *const vec, const size_t index)                            \
    {                                                                          \
        if (index >= vec->length) {                                            \
            fprintf(stderr,                                                    \
                "%s: Cannot remove at index %zu from length %zu vec\n",        \
                __func__, index, vec->length);                                 \
                                                                               \
            exit(1);                                                           \
        }                                                                      \
                                                                               \
        type result = vec->buf[index];                                         \
        vec->length -= 1;                                                      \
        for (size_t i = index; i < vec->length; i += 1) {                      \
            vec->buf[i] = vec->buf[i + 1];                                     \
        }                                                                      \
                                                                               \
        return result;                                                         \
    }                                                                          \
                                                                               \
    type vec##suffix##_remove_at_swap(                                         \
        vec##suffix *const vec, const size_t index)                            \
    {                                                                          \
        if (index >= vec->length) {                                            \
            fprintf(stderr,                                                    \
                "%s: Cannot remove at index %zu from length %zu vec\n",        \
                __func__, index, vec->length);                                 \
                                                                               \
            exit(1);                                                           \
        }                                                                      \
                                                                               \
        type result = vec->buf[index];                                         \
        vec->buf[index] = vec->buf[vec->length - 1];                           \
        vec->length -= 1;                                                      \
        return result;                                                         \
    }                                                                          \
                                                                               \
    void vec##suffix##_shrink_to_fit(vec##suffix *const vec)                   \
    {                                                                          \
        vec->capacity = vec->length;                                           \
        vec##suffix##_reallocate_buf(vec);                                     \
    }

// Generates both header and implementation code for the given type.
// Convenience macro for use in a place like the file with the main function.
// See above for explanation of suffix.
#define VEC_GENERATE_FOR_TYPE(type, suffix)                                    \
    VEC_GENERATE_HEADER_CODE(type, suffix)                                     \
    VEC_GENERATE_IMPLEMENTATION_CODE(type, suffix)

////////////////////////////////////////////////////////////////////////////////

#endif // VEC_H

////////////////////////////////////////////////////////////////////////////////

// Other possible functions:
// reserve_at_shift
// reserve_at_swap

// Note:
// Structs cannot be compared with ==
// This means that we cannot generate functions that would be expected to
//   find a certain value in the buffer, unless we are able to selectively
//   not generate these functions when the type is a struct.
//
// It would be possible to communicate some way that a certain struct
//   should be compared for equality (or a special equality check for any
//   certain type), but probably would not be worth the complexity -- just
//   accomplish your task by iterating across the buffer instead.

////////////////////////////////////////////////////////////////////////////////

/*
BSD 2-Clause License

Copyright 2021 Costava

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
