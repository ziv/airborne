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
#include "GameData.h"
#include "raylib.h"

/// @brief A single waypoint in the autopilot route.
struct Waypoint {
    Vector3 Position;      ///< Target world-space position.
    float TargetSpeed;     ///< Desired speed (m/s) when reaching this waypoint.
    float ArrivalRadius;   ///< Distance (meters) at which the waypoint is considered reached.
};

/**
 * @brief Waypoint-following autopilot that outputs PilotControls.
 *
 * Uses a "bank and pull" manoeuvre: it rolls to point the lift vector toward
 * the target heading and then pitches to match the desired climb/descent angle.
 */
class Autopilot {
    std::vector<Waypoint> route;
    size_t currentWaypointIndex = 0;

    float maxBankAngle = 45.0f;   ///< Maximum roll angle (degrees) the autopilot will command.
    float maxPullRatio = 1.0f;    ///< Pitch aggression factor during banked turns.
    float speedRatio = 1.0f;      ///< Throttle change rate.
    bool active = false;

public:
    Autopilot(float maxBankAngle,
              float maxPullRatio,
              float speedRatio);

    void Toggle() { active = !active; }

    /// @brief Append a waypoint to the end of the route.
    void AddWaypoint(const Vector3 &position,
                     float targetSpeed,
                     float arrivalRadius);

    [[nodiscard]] bool IsActive() const;

    /// @brief Compute one frame of steering commands toward the current waypoint.
    /// @return PilotControls with roll, pitch, throttle deltas for this frame.
    // todo convert this list of argument to struct
    PilotControls Steer(const Vector3 &position,
                        const Vector3 &forward,
                        const Vector3 &right,
                        const Vector3 &up,
                        float deltaTime,
                        float speed);
};
