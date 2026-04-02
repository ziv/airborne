#pragma once
#include "AircraftCamera.h"
#include "AircraftControls.h"
#include "AircraftPhysics.h"
#include "AppConfig.h"

class GameData {
    Meter heightAboveGround;
    MeterPerSecond stallSpeed;

public:
    bool paused = false;

    // aircraft controllers
    AircraftControls aircraftControls;
    AircraftPhysics aircraftPhysics;
    AircraftTransformation aircraftTransformation;
    AircraftCamera aircraftCamera;

    explicit GameData(const AppConfig &config);

    void update();
};
