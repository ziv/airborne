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

    /// Landing zone box height above surface (how tall the 3D box is).
    static constexpr float LANDING_BOX_HEIGHT = 150.0f;

    /// Maximum landing speed — anything faster causes a crash on touchdown.
    static constexpr float MAX_LANDING_SPEED_RATIO = 2.0f;

    /// Maximum roll angle (degrees) for a safe landing.
    static constexpr float MAX_LANDING_ROLL = 12.0f;

    /// Pitch must be between these bounds (degrees) for a safe landing.
    static constexpr float MIN_LANDING_PITCH = -10.0f;
    static constexpr float MAX_LANDING_PITCH = 15.0f;

    /// Check if the aircraft is inside any friendly AIRBASE landing zone.
    /// Populates LandingZoneInfo with zone details if found.
    [[nodiscard]] LandingZoneInfo checkLandingZones() const;

    /// Returns true when orientation and speed allow a safe touchdown.
    [[nodiscard]] bool isGoodLanding() const;

    /// Spawns all entities defined in the current scenario.
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

    /// Construct with a scenario — every mission starts on the ground.
    GameData(const AppConfig &config, const Scenario &scenario);

    void update(float dt);
};
