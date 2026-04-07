#pragma once
#include "../primitives/AppConfig.h"
#include "AircraftStructs.h"
#include "../primitives/Types.h"


/// @brief Holds the direction/orientation of the aircraft in 3d space
class AircraftTransformation {
    // configuration
    MeterPerSecond maxSpeed;
    MeterPerSecond vleSpeed;
    MeterPerSecond stallSpeed;
    Ratio bankInduceYawRatio;
    Ratio liftLossPitchRatio;

    void flyingOrientation(AircraftState &state, float dt) const;

    void groundOrientation(AircraftState &state, float dt) const;

    static void updateOrientation(AircraftState &state, const Quaternion &qDelta) ;

public:
    explicit AircraftTransformation(const AppConfig &config);

    /// @brief apply transformation on aircraft state
    void update(AircraftState &state, float dt) const;
};
