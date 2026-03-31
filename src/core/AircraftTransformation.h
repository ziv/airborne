#pragma once
#include "AircraftControls.h"
#include "raylib.h"

struct Directions {
    Vector3 forward;
    Vector3 right;
    Vector3 up;
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
    Vector3 forward{0};
    Vector3 up{0};
    Vector3 right{0};

    void flyingOrientation(float dt, MeterPerSecond speed, const PilotControls &controls);

    void groundOrientation(MeterPerSecond speed, const PilotControls &controls);

    void recalculateDirectionVectors();

public:
    explicit AircraftTransformation(const AppConfig &config);

    void update(float dt, bool flying, const PilotControls &controls, MeterPerSecond speed);

    [[nodiscard]] Quaternion getRotation() const;

    [[nodiscard]] Directions getDirections() const;
};
