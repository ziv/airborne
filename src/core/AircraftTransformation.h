#pragma once
#include "AircraftControls.h"
#include "raylib.h"
#include "../primitives/Constants.h"

struct Directions {
    Vector3 forward;
    Vector3 up;
    Vector3 right;
};

/**
 * Holds the direction of the aircraft in 3d space
 */
class AircraftTransformation {
    // configuration
    MeterPerSecond maxSpeed;
    MeterPerSecond vleSpeed;
    MeterPerSecond stallSpeed;
    float bankInduceYawRatio;
    float liftLossPitchRatio;

    // internal state
    Quaternion rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    Vector3 forward = GamePhysics::WorldForward;
    Vector3 up = GamePhysics::WorldUp;
    Vector3 right = GamePhysics::WorldRight;

    void flyingOrientation(float dt, MeterPerSecond speed, const PilotControls &controls);

    void groundOrientation(MeterPerSecond speed, const PilotControls &controls);

    void recalculateDirectionVectors();

public:
    explicit AircraftTransformation(const AppConfig &config);

    void update(float dt, bool flying, const PilotControls &controls, MeterPerSecond speed);

    [[nodiscard]] Quaternion getRotation() const;

    [[nodiscard]] Directions getDirections() const;
};
