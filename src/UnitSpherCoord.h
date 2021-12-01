#ifndef UNITSPHERCOORD_H
#define UNITSPHERCOORD_H

#include "math.h"

#include "M_PI.h"
#include "v3d.h"

// Unit-radius spherical coordinates.
typedef struct UnitSpherCoord {
    // Azimuthal angle is on xy plane.
    double azim;
    // Inclination angle.
    double incl;
} UnitSpherCoord;

static inline v3d UnitSpherCoord_ToCartesian(const UnitSpherCoord spher)
{
    return (v3d) {
        sin(spher.incl) * cos(spher.azim),
        sin(spher.incl) * sin(spher.azim),
        cos(spher.incl)
    };
}

static inline UnitSpherCoord UnitSpherCoord_FromCartesian(v3d point) {
    point = v3d_Unit(point);
    return (UnitSpherCoord) {
        .azim = atan2(point.y, point.x) + 2.0 * M_PI * (point.y < 0),
        .incl = acos(point.z)
	};
}

#endif // UNITSPHERCOORD_H
