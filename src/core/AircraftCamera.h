#pragma once
#include "AircraftTransformation.h"
#include "../primitives/AppConfig.h"
#include "raylib.h"

class AircraftCamera {
    // configuration
    float pilotTilt;
    float heightAboveGround;

    // state
    Camera camera = {0};

public:
    explicit AircraftCamera(const AppConfig &config);

    void update(AircraftState &state, float dt);

    void setPosition(const Vector3 &position);

    Camera &getCamera() { return camera; }
};
