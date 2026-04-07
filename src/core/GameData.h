#pragma once
#include "AircraftCamera.h"
#include "AircraftControls.h"
#include "AircraftPhysics.h"
#include "../primitives/AppConfig.h"
#include "../scenario/Scenario.h"
#include "../entities/EntityRegistry.h"
#include "../entities/GroundTarget.h"

class GameData {
    Meter heightAboveGround;
    MeterPerSecond stallSpeed;

    [[nodiscard]] bool isStableLanding();
    void spawnInitialEntities();

public:
    bool paused = false;
    AircraftState state;
    Scenario scenario;
    EntityRegistry entities;

    // aircraft controllers
    AircraftControls aircraftControls;
    AircraftPhysics aircraftPhysics;
    AircraftTransformation aircraftTransformation;
    AircraftCamera aircraftCamera;

    explicit GameData(const AppConfig &config);
    GameData(const AppConfig &config, const Scenario &scenario);

    void update(float dt);
};
