#pragma once
#include "../primitives/AppConfig.h"
#include "../primitives/Types.h"
#include "AircraftStructs.h"


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

public:
    explicit AircraftPhysics(const AppConfig &config);

    /// @brief apply physics on aircraft state
    void update(AircraftState &state, float dt) const;
};
