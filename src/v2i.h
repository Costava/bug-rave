// Vector struct with 2 int fields.

#ifndef V2I_H
#define V2I_H

#include <stdbool.h>

typedef struct v2i {
    int x;
    int y;
} v2i;

typedef struct MaybeV2i {
    bool hasValue;
    v2i  value;
} MaybeV2i;

// Return a - b
static inline struct v2i v2i_Sub(const struct v2i a, const struct v2i b) {
    return (struct v2i) {
        .x = a.x - b.x,
        .y = a.y - b.y
    };
}

// Return a with all members multiplied by mul
static inline struct v2i v2i_Mul(const struct v2i a, const double mul) {
    return (struct v2i) {
        // TODO: Something better than (int) cast?
        .x = a.x * (int)mul,
        .y = a.y * (int)mul
    };
}

// Return dot product of a and b
static inline int v2i_Dot(const struct v2i a, const struct v2i b) {
    return a.x * b.x + a.y * b.y;
}

static inline v3d v2i_GetBary(v2i t1, v2i t2, v2i t3, v2i point)
{
    v2i v0 = v2i_Sub(t2, t1);
    v2i v1 = v2i_Sub(t3, t1);
    v2i v2 = v2i_Sub(point, t1);

    const int d00 = v2i_Dot(v0, v0);
    const int d01 = v2i_Dot(v0, v1);
    const int d11 = v2i_Dot(v1, v1);
    const int d20 = v2i_Dot(v2, v0);
    const int d21 = v2i_Dot(v2, v1);

    const double denom = (double)(d00 * d11 - d01 * d01);

    const double y = (double)(d11 * d20 - d01 * d21) / denom;
    const double z = (double)(d00 * d21 - d01 * d20) / denom;

    return (v3d) {
        .x = 1.0 - y - z,
        .y = y,
        .z = z};
}

#endif // V2I_H
