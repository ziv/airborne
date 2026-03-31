#pragma once
#include "AircraftTransformation.h"
#include "AppConfig.h"
#include "raylib.h"

class AircraftCamera {
    // configuration
    float pilotTilt;
    float heightAboveGround;

    // state
    Camera camera = {0};

public:
    explicit AircraftCamera(const AppConfig &config);

    void update(float dt, const Directions &directions, const Vector3 &velocity);

    void setPosition(const Vector3 &position);

    Camera &getCamera() { return camera; }
};
