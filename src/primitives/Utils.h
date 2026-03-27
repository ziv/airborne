#pragma once
#include "raylib.h"

inline Vector3 GetFlatForward(const Vector3 &currentForward, const Vector3 &currentUp) {
    Vector3 flatForward = {currentForward.x, 0.0f, currentForward.z};

    // edge case guard
    if (Vector3Length(flatForward) < 0.001f) {
        // const Vector3 currentUp = Vector3RotateByQuaternion(GamePhysics::WorldUp, rotation);
        flatForward = {currentUp.x, 0.0f, currentUp.z};
    }
    return flatForward;
}
