#pragma once
#include "AircraftTransformation.h"
#include "AppConfig.h"
#include "../primitives/Types.h"
#include "raylib.h"

class AircraftPhysics {
    // those values come from configuration
    Newton weight;
    Newton engineThrust;
    MeterPerSecond maxSpeed;
    MeterPerSecond stallSpeed;
    float dragCoefficient;
    float liftCoefficient;

    // internal state
    Newton thrust = 0.0_ntn;
    Newton drag = 0.0_ntn;
    Newton lift = 0.0_ntn;
    MeterPerSecond speed = 0.0_mps;
    Vector3 velocity = {0.0f, 0.0f, 0.0f};

public:
    explicit AircraftPhysics(const AppConfig &config);

    // update the state
    void update(float dt, bool flying, const PilotControls &controls, const Directions &dir);

    // void setForces(Newton t, Newton d, Newton l);

    [[nodiscard]] Newton getThrust() const { return thrust; }
    [[nodiscard]] Newton getDrag() const { return drag; }
    [[nodiscard]] Newton getLift() const { return lift; }
    [[nodiscard]] MeterPerSecond getSpeed() const { return speed; }
    [[nodiscard]] Vector3 getVelocity() const { return velocity; }
};
