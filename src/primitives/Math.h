#pragma once
#include "raylib.h"

inline float SquareDistance(const Vector3 v1, const Vector3 v2) {
    const auto dx = v1.x - v2.x;
    const auto dy = v1.y - v2.y;
    const auto dz = v1.z - v2.z;
    return dx * dx + dy * dy + dz * dz;
}

inline float SquareDistance(const Vector2 v1, const Vector2 v2) {
    const auto dx = v1.x - v2.x;
    const auto dy = v1.y - v2.y;
    return dx * dx + dy * dy;
}