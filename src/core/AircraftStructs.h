#pragma once
#include "raylib.h"
#include "../primitives/Constants.h"
#include "../primitives/Types.h"

struct PilotControls {
    float pitch;
    float yaw;
    float roll;
    float throttle;
    bool brakes;
    bool gear;
};

struct PhysicsForces {
    Newton thrust;
    Newton drag;
    Newton lift;
    MeterPerSecond speed;
    Vector3 velocity;
};

struct Orientation {
    Quaternion rotation;
    Vector3 forward;
    Vector3 up;
    Vector3 right;
};

struct AircraftState {
    PilotControls controls = {
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        true,
        true
    };
    PhysicsForces forces = {
        0.0_ntn,
        0.0_ntn,
        0.0_ntn,
        0.0_mps,
        {0.0f, 0.0f, 0.0f}
    };
    Orientation orientation = {
        {0.0f, 0.0f, 0.0f, 1.0f},
        GamePhysics::WorldForward,
        GamePhysics::WorldUp,
        GamePhysics::WorldRight
    };
    Vector3 position = {
        0.0f,
        0.0f,
        0.0f
    };
    float groundHeight = 0;
    bool flying = false;
    bool crushed = false;
    float fuel = 3500;
};
