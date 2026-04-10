/**
 * @file Autopilot.h
 * @brief Waypoint-following autopilot using a bank-and-pull steering algorithm.
 *
 * The autopilot follows a pre-defined route of waypoints. At each frame it
 * computes roll, pitch, and throttle commands to steer the aircraft toward the
 * next waypoint while respecting configurable bank angle, pull rate, and speed
 * limits.
 */
#pragma once
#include <vector>
#include "AircraftStructs.h"
#include "raylib.h"
#include "../primitives/AppConfig.h"
#include "../scenario/Scenario.h"

/// @brief A single waypoint in the autopilot route.
struct Waypoint {
    std::string name;
    Vector3 position; ///< Target world-space position.
    float targetSpeed;
    float arrivalRadius;
};

/**
 * @brief Waypoint-following autopilot that outputs PilotControls.
 *
 * Uses a "bank and pull" manoeuvre: it rolls to point the lift vector toward
 * the target heading and then pitches to match the desired climb/descent angle.
 */
class Autopilot {
    std::vector<Waypoint> route;
    int currentWaypointIndex = 0;

    float maxBankAngle = 45.0f; ///< Maximum roll angle (degrees) the autopilot will command.
    float maxPullRatio = 1.0f; ///< Pitch aggression factor during banked turns.
    float speedRatio = 1.0f; ///< Throttle change rate.
    bool active = false;

public:
    Autopilot(const AppConfig &config, const Scenario &scenario);

    [[nodiscard]] bool isActive() const;

    void toggle();

    /// @brief Append a waypoint to the end of the route.
    void addWaypoint(const std::string &name, const Vector3 &position, float targetSpeed, float arrivalRadius);

    /// @brief Steer the aircraft by mutating its controls.
    void steer(AircraftState &state, float deltaTime);
};
