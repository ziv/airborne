#pragma once
#include "raylib.h"
#include "../primitives/Types.h"

/// @brief all the forces act on an aircraft
/// @brief speed is not a force, it is the size if the velocity vector
struct Forces {
    Newton thrust;
    Newton drag;
    Newton lift;
    Vector3 velocity;
    MeterPerSecond speed;
};

struct AircraftProperties {
    Newton weight;
    Newton engineThrust;
    MeterPerSecond maxSpeed;
    MeterPerSecond stallSpeed;
    MeterPerSecond groundBrakesSpeed;
    Ratio dragCoefficient;
    Ratio liftCoefficient;

    float bankInduceYawRatio;
    float liftLossPitchRatio;
};
