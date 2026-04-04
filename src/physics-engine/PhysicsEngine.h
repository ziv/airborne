#pragma once
#include "Primitives.h"
#include "../core/AircraftStructs.h"

class PhysicsEngine {
    AircraftProperties props;
    // Newton weight;
    // Newton engineThrust;
    // MeterPerSecond maxSpeed;
    // MeterPerSecond stallSpeed;
    // MeterPerSecond groundBrakesSpeed;
    // Ratio dragCoefficient;
    // Ratio liftCoefficient;

    // Ratio flyingBrakesDragRatio;
    // Ratio flyingGearDragRatio;
    // Ratio groundBrakesDragRatio;
    // Ratio stallLiftRatio;

public:
    explicit PhysicsEngine(const AircraftProperties &props);

    void applyFlying(const AircraftState &state, float dt, float dragRatio, float liftRatio) const;
};
