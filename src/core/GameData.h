#pragma once
#include "AircraftCamera.h"
#include "AircraftControls.h"
#include "AircraftPhysics.h"
#include "../primitives/AppConfig.h"

class GameData {
    Meter heightAboveGround;
    MeterPerSecond stallSpeed;
    // PhysicsEngine pysicsEngine;

    [[nodiscard]] bool isStableLanding();
public:
    bool paused = false;
    AircraftState state;

    // aircraft controllers
    AircraftControls aircraftControls;
    AircraftPhysics aircraftPhysics;
    AircraftTransformation aircraftTransformation;
    AircraftCamera aircraftCamera;

    explicit GameData(const AppConfig &config);

    void update(float dt);
};
