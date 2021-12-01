// Vector with 2 double fields

#ifndef V2D_H
#define V2D_H

#include <math.h> // sqrt
#include <stdbool.h>

#include "v3d.h"

typedef struct v2d {
    double x;
    double y;
} v2d;

typedef struct MaybeV2d {
    bool hasValue;
    v2d  value;
} MaybeV2d;

static inline struct v2d v2d_FromV3d(const v3d v) {
    return (v2d) {
        .x = v.x,
        .y = v.y
    };
}

// Return a + b
static inline struct v2d v2d_Add(const struct v2d a, const struct v2d b) {
    return (struct v2d) {
        .x = a.x + b.x,
        .y = a.y + b.y
    };
}

// Return a - b
static inline struct v2d v2d_Sub(const struct v2d a, const struct v2d b) {
    return (struct v2d) {
        .x = a.x - b.x,
        .y = a.y - b.y
    };
}

// Return a with all members multiplied by mul
static inline struct v2d v2d_Mul(const struct v2d a, const double mul) {
    return (struct v2d) {
        .x = a.x * mul,
        .y = a.y * mul
    };
}

// Return a with all members divided by div
static inline struct v2d v2d_Div(const struct v2d a, const double div) {
    return (struct v2d) {
        .x = a.x / div,
        .y = a.y / div
    };
}

// Return magnitude of a
static inline double v2d_Mag(const struct v2d a) {
    return sqrt(a.x * a.x + a.y * a.y);
}

// Return unit vector in direction of a
static inline struct v2d v2d_Unit(const struct v2d a) {
    const double mag = v2d_Mag(a);
    return (struct v2d) {
        .x = a.x / mag,
        .y = a.y / mag
    };
}

// Return midpoint of a and b
static inline struct v2d v2d_Midpoint(const struct v2d a, const struct v2d b) {
    return v2d_Div(v2d_Add(a, b), 2.0);
}

// Return dot product of a and b
static inline double v2d_Dot(const struct v2d a, const struct v2d b) {
    return a.x * b.x + a.y * b.y;
}

// Return distance between a and b
static inline double v2d_Distance(const struct v2d a, const struct v2d b) {
    return v2d_Mag(v2d_Sub(a, b));
}

static inline v3d v2d_GetBary(v2d t1, v2d t2, v2d t3, v2d point)
{
    v2d v0 = v2d_Sub(t2, t1);
    v2d v1 = v2d_Sub(t3, t1);
    v2d v2 = v2d_Sub(point, t1);

    const double d00 = v2d_Dot(v0, v0);
    const double d01 = v2d_Dot(v0, v1);
    const double d11 = v2d_Dot(v1, v1);
    const double d20 = v2d_Dot(v2, v0);
    const double d21 = v2d_Dot(v2, v1);

    const double denom = d00 * d11 - d01 * d01;

    const double y = (d11 * d20 - d01 * d21) / denom;
    const double z = (d00 * d21 - d01 * d20) / denom;

    return (v3d) {
        .x = 1.0 - y - z,
        .y = y,
        .z = z};
}

#endif // V2D_H
