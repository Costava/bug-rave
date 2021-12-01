#ifndef V3D
#define V3D

// Vector with 3 double fields

#include <math.h> // sqrt

typedef struct v3d {
    double x;
    double y;
    double z;
} v3d;

typedef struct MaybeV3d {
    bool hasValue;
    v3d  value;
} MaybeV3d;

// Return a + b
static inline struct v3d v3d_Add(const struct v3d a, const struct v3d b) {
    return (struct v3d) {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z
    };
}

// Return a - b
static inline struct v3d v3d_Sub(const struct v3d a, const struct v3d b) {
    return (struct v3d) {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z
    };
}

// Return a with all members multiplied by mul
static inline struct v3d v3d_Mul(const struct v3d a, const double mul) {
    return (struct v3d) {
        .x = a.x * mul,
        .y = a.y * mul,
        .z = a.z * mul
    };
}

// Return a with all members divided by div
static inline struct v3d v3d_Div(const struct v3d a, const double div) {
    return (struct v3d) {
        .x = a.x / div,
        .y = a.y / div,
        .z = a.z / div
    };
}

// Return a with components subtracted from 0.
static inline struct v3d v3d_Invert(const struct v3d a) {
    return (struct v3d) {
        .x = 0.0 - a.x,
        .y = 0.0 - a.y,
        .z = 0.0 - a.z
    };
}

static inline struct v3d v3d_Cross(const struct v3d a, const struct v3d b) {
    return (v3d) {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

// Return magnitude of a
static inline double v3d_Mag(const struct v3d a) {
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

// Return magnitude of a
// Ignore z field
static inline double v3d_Mag2(const struct v3d a) {
    return sqrt(a.x * a.x + a.y * a.y);
}

// Return unit vector in direction of a
static inline struct v3d v3d_Unit(const struct v3d a) {
    const double mag = v3d_Mag(a);
    return (struct v3d) {
        .x = a.x / mag,
        .y = a.y / mag,
        .z = a.z / mag
    };
}

// Return a scaled to have given magnitude.
static inline v3d v3d_SetMag(const struct v3d a, const double mag) {
    return v3d_Mul(v3d_Unit(a), mag);
}

// Return midpoint of a and b
static inline struct v3d v3d_Midpoint(const struct v3d a, const struct v3d b) {
    return v3d_Div(v3d_Add(a, b), 2.0);
}

// Return dot product of a and b
static inline double v3d_Dot(const struct v3d a, const struct v3d b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Return dot product of a and b
// Ignore z field
static inline double v3d_Dot2(const struct v3d a, const struct v3d b) {
    return a.x * b.x + a.y * b.y;
}

// Return distance between a and b
static inline double v3d_Distance(const struct v3d a, const struct v3d b) {
    return v3d_Mag(v3d_Sub(a, b));
}

// Return distance between a and b
// Ignore z field
static inline double v3d_Distance2(const struct v3d a, const struct v3d b) {
    return v3d_Mag2(v3d_Sub(a, b));
}

static inline v3d v3d_GetBary(v3d t1, v3d t2, v3d t3, v3d point)
{
    v3d v0 = v3d_Sub(t2, t1);
    v3d v1 = v3d_Sub(t3, t1);
    v3d v2 = v3d_Sub(point, t1);

    const double d00 = v3d_Dot(v0, v0);
    const double d01 = v3d_Dot(v0, v1);
    const double d11 = v3d_Dot(v1, v1);
    const double d20 = v3d_Dot(v2, v0);
    const double d21 = v3d_Dot(v2, v1);

    const double denom = d00 * d11 - d01 * d01;

    double y = (d11 * d20 - d01 * d21) / denom;
    double z = (d00 * d21 - d01 * d20) / denom;
    double x = 1.0 - y - z;

    return (v3d) { x, y, z };
}

// Ignore z field
static inline v3d v3d_GetBary2(v3d t1, v3d t2, v3d t3, v3d point)
{
    v3d v0 = v3d_Sub(t2, t1);
    v3d v1 = v3d_Sub(t3, t1);
    v3d v2 = v3d_Sub(point, t1);

    const double d00 = v3d_Dot2(v0, v0);
    const double d01 = v3d_Dot2(v0, v1);
    const double d11 = v3d_Dot2(v1, v1);
    const double d20 = v3d_Dot2(v2, v0);
    const double d21 = v3d_Dot2(v2, v1);

    const double denom = d00 * d11 - d01 * d01;

    double y = (d11 * d20 - d01 * d21) / denom;
    double z = (d00 * d21 - d01 * d20) / denom;
    double x = 1.0 - y - z;

    return (v3d) { x, y, z };
}

static inline v3d v3d_RotateZ(const v3d v, const double rad) {
    double angle = atan2(v.y, v.x);
    double mag = v3d_Mag2(v);
    angle += rad;

    return (v3d) {
        .x = cos(angle) * mag,
        .y = sin(angle) * mag,
        .z = v.z
    };
}

static inline v3d v3d_Lerp(double l, v3d a, v3d b) {
    return v3d_Add(v3d_Mul(a, 1.0 - l), v3d_Mul(b, l));
}

#endif // V3D
