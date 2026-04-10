/**
 * @file GameData.h
 * @brief Top-level simulation orchestrator — composes controls, physics,
 *        transformation, camera, and entity management.
 *
 * Each frame, GameData::update() runs the full simulation pipeline in order:
 *   1. Landing zone detection
 *   2. Floor height calculation
 *   3. Flying/ground state transition
 *   4. Controls → Physics → Crash detection → Transformation → Camera → Entities
 */
#pragma once
#include "AircraftCamera.h"
#include "AircraftControls.h"
#include "AircraftPhysics.h"
#include "Autopilot.h"
#include "../primitives/AppConfig.h"
#include "../scenario/Scenario.h"
#include "../entities/EntityRegistry.h"
#include "../entities/GroundTarget.h"

/**
 * @brief Orchestrates the per-frame aircraft simulation and entity management.
 *
 * Owns all simulation subsystems and the entity registry. Handles landing-zone
 * detection, ground collision, and crash logic as part of the update cycle.
 */
class GameData {
    Meter heightAboveGround;
    MeterPerSecond stallSpeed;

    /// Landing zone box height above surface (how tall the 3D box is).
    static constexpr float LANDING_BOX_HEIGHT = 150.0f;

    /// Maximum landing speed — anything faster causes a crash on touchdown.
    static constexpr float MAX_LANDING_SPEED_RATIO = 2.0f;

    /// Maximum roll angle (degrees) for a safe landing.
    static constexpr float MAX_LANDING_ROLL = 10.0f;

    /// Pitch must be between these bounds (degrees) for a safe landing.
    static constexpr float MIN_LANDING_PITCH = -5.0f;
    static constexpr float MAX_LANDING_PITCH = 20.0f;

    /// Check if the aircraft is inside any friendly AIRBASE landing zone.
    /// Populates LandingZoneInfo with zone details if found.
    [[nodiscard]] LandingZoneInfo checkLandingZones() const;

    /// Returns true when orientation and speed allow a safe touchdown.
    [[nodiscard]] bool isGoodLanding() const;

    /// Spawns all entities defined in the current scenario.
    void spawnInitialEntities();

public:
    Autopilot autopilot;
    bool paused = false;          ///< When true the simulation is frozen.
    AircraftState state;          ///< Central mutable aircraft state.
    Scenario scenario;            ///< Active mission definition.
    EntityRegistry entities;      ///< All spawned game entities.

    // --- Aircraft simulation subsystems (executed in order) ---
    AircraftControls aircraftControls;
    AircraftPhysics aircraftPhysics;
    AircraftTransformation aircraftTransformation;
    AircraftCamera aircraftCamera;

    /// @brief Construct and initialize all subsystems; places the aircraft at the scenario start.
    GameData(const AppConfig &config, const Scenario &scenario);

    /// @brief Run one simulation frame (controls → physics → orientation → camera → entities).
    /// @param dt Frame delta time (seconds).
    void update(float dt);
};
