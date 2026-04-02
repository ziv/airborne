#pragma once
#include "AppConfig.h"
#include "AircraftStructs.h"

class AircraftControls {
    // configuration
    float pitchRatio;
    float rollRatio;
    float yawRatio;

    // state
    PilotControls controls = {0.0f, 0.0f, 0.0f, 0.0f, true, true};

public:
    explicit AircraftControls(const AppConfig &config);

    [[nodiscard]] PilotControls &getControls();

    void update(float dt);
};
