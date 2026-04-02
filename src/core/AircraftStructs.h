#pragma once
#include "raylib.h"
#include "../primitives/Types.h"

struct PilotControls {
    float pitch;
    float yaw;
    float roll;
    float throttle;
    bool brakes;
    bool gear;
};

struct ForcesState {
    Newton thrust;
    Newton drag;
    Newton lift;
    MeterPerSecond speed;
    Vector3 velocity;
};

struct Directions {
    Vector3 forward;
    Vector3 up;
    Vector3 right;
};
