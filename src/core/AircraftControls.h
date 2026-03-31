#pragma once
#include "AppConfig.h"

struct PilotControls {
    float pitch;
    float yaw;
    float roll;
    float throttle;
    bool brakes;
    bool gear;
};

class AircraftControls {
    float pitchRatio;
    float rollRatio;
    float yawRatio;

    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = 0.0f;
    float throttle = 0.0f;

    bool brakes = true;
    bool gear = true;

public:
    explicit AircraftControls(const AppConfig &config);

    [[nodiscard]] PilotControls getControls() const;

    void update(float dt);
};
