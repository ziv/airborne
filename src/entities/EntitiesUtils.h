#pragma once
#include <cmath>
#include "raylib.h"

inline bool AircraftInLandingBox(const Vector3 &basePos,
                                 const Vector3 &aircraftPos,
                                 const float length,
                                 const float width,
                                 const float heading) {
    const float dx = basePos.x - aircraftPos.x;
    const float dz = basePos.z - aircraftPos.z;
    const float rad = heading * DEG2RAD;
    const float cosH = std::cosf(rad);
    const float sinH = std::sinf(rad);
    const float localAlong = dx * sinH + dz * cosH; // along runway
    const float localAcross = dx * cosH - dz * sinH; // across runway

    return fabsf(localAlong) < length / 2 && fabsf(localAcross) < width / 2;
}
