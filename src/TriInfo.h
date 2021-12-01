#ifndef TRIINFO_H
#define TRIINFO_H

#include "v2d.h"
#include "v3d.h"
#include "vec.h"

typedef struct TriInfoSet {
    v3d world;
    // Third dimension is depth (distance in front of camera)
    v3d screen;
    v2d tex;
} TriInfoSet;

typedef struct TriInfo {
    TriInfoSet set[3];
} TriInfo;

VEC_GENERATE_HEADER_CODE(TriInfo, triinfo)

#endif // TRIINFO_H
