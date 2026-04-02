#pragma once
#include "AppConfig.h"
#include "AircraftStructs.h"
#include "../primitives/Types.h"
#include "raylib.h"


class AircraftPhysics {
    // configuration
    Newton weight;
    Newton engineThrust;
    MeterPerSecond maxSpeed;
    MeterPerSecond stallSpeed;
    MeterPerSecond groundBrakesSpeed;
    Ratio dragCoefficient;
    Ratio liftCoefficient;
    Ratio flyingBrakesDragRatio;
    Ratio flyingGearDragRatio;
    Ratio groundBrakesDragRatio;
    Ratio stallLiftRatio;

    // internal state
    ForcesState forces = {0.0_ntn, 0.0_ntn, 0.0_ntn, 0.0_mps, {0.0f, 0.0f, 0.0f}};

public:
    explicit AircraftPhysics(const AppConfig &config);

    // update the state
    void update(float dt, bool flying, const PilotControls &controls, const Directions &dir);

    [[nodiscard]] ForcesState &getForces() { return forces; }
};
