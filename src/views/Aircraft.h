/**
 * @file Aircraft.h
 * @brief External 3D model of the player's F-15 — used for the chase-camera view.
 *
 * When the player holds F1 the cockpit is hidden and this model is drawn at
 * the aircraft's position so the player can see the aircraft from outside.
 */
#pragma once
#include <vector>
#include "../core/AircraftStructs.h"
#include "../primitives/Resource.h"
#include "../primitives/Types.h"

/// @brief Behavioural mode for AI aircraft (reserved for future use).
enum AircraftMode {
    Patrol,
    Engage,
    Flee
};

/// @brief Waypoint for AI path-following (reserved for future use).
struct AircraftWaypoint {
    Vector3 position;
    float speed;
    float arrivalRadius;
};

/// @brief Steering output for AI aircraft (reserved for future use).
struct AircraftSteer {
    float roll;
    float pitch;
    float throttle;
};

/// @brief Renders the player's aircraft model from an external perspective.
class Aircraft {
private:
    // ModelHandle aircraft;
    // limit the roll
    // float maxBankAngle = 45.0f;
    //
    // // limit the pitch
    // float maxPullRatio = 1.0f;
    //
    // // limit the throttle
    // float speedRatio = 1.0f;
    // float maxTurnRate = 0.5f;
    //
    //
    // MeterPerSecond speed = 10.0;
    // Vector3 forward = {-1.0f, 0.0f, 0.0f};

    // ModelHandle model;

    // std::pmr::vector<AircraftWaypoint> waypoints;
    // size_t currentWaypointIndex = 0;

    // void steer(float dt);

public:
    // Vector3 position = {500.0f, 1000.0f, 500.0f};
    Aircraft();

    // void update(const AircraftState &state, float dt);

    void draw(const AircraftState &state);
};
