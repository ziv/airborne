#pragma once
#include "raylib.h"
#include "raymath.h"
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
    // to keep floating point precision to work in great distances
    Vector3 worldOffset = {0.0f, 0.0f, 0.0f};
    const float SHIFT_THRESHOLD = 5000.0f;

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
    Vector2 mapOffset = {0.0f, 0.0f};
    // Vector3 localPosition = {
    //     0.0f,
    //     0.0f,
    //     0.0f
    // };
    float groundHeight = 0;
    bool flying = false;
    bool crushed = false;
    float fuel = 3500;


    // we draw only items in 50,000m radius
    [[nodiscard]] bool tooFar2Draw(const Vector3 &pos) const {
        const auto mine = Vector3Add(position, Vector3({mapOffset.x, 0.0f, mapOffset.y}));
        const auto distance = sqrtf((mine.x - pos.x) * (mine.x - pos.x) + (mine.y - pos.y) * (mine.y - pos.y));
        return distance > 50000;
    }
};
