#pragma once
#include "AircraftTransformation.h"
#include "../primitives/AppConfig.h"
#include "raylib.h"

class AircraftCamera {
    // configuration
    float pilotTilt;

    // state
    Camera camera = {0};

public:
    explicit AircraftCamera(const AppConfig &config);

    /// @brief update camera position and target
    void update(const AircraftState &state, float dt);

    Camera &getCamera() { return camera; }
};
