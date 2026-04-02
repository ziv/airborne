#pragma once
#include "AppConfig.h"
#include "raylib.h"
#include "AircraftStructs.h"
#include "../primitives/Constants.h"
#include "../primitives/Types.h"


/**
 * Holds the direction/orientation of the aircraft in 3d space
 */
class AircraftTransformation {
    // configuration
    MeterPerSecond maxSpeed;
    MeterPerSecond vleSpeed;
    MeterPerSecond stallSpeed;
    Ratio bankInduceYawRatio;
    Ratio liftLossPitchRatio;

    // internal state
    Quaternion rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    Directions dir = {GamePhysics::WorldForward, GamePhysics::WorldUp, GamePhysics::WorldRight};

    void flyingOrientation(float dt, MeterPerSecond speed, const PilotControls &controls);

    void groundOrientation(MeterPerSecond speed, const PilotControls &controls);

    void recalculateDirectionVectors();

public:
    explicit AircraftTransformation(const AppConfig &config);

    void update(float dt, bool flying, const PilotControls &controls, const ForcesState &forces);

    [[nodiscard]] Quaternion &getRotation();

    [[nodiscard]] Directions &getDirections();
};
