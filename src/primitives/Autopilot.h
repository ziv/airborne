#pragma once
#include <vector>
#include "GameCamera.h"
#include "raylib.h"

struct Waypoint {
    Vector3 Position;
    float TargetSpeed;
};

class Autopilot {
    std::vector<Waypoint> route;
    size_t currentWaypointIndex = 0;
    float arrivalRadius = 100.0f;

public:
    Autopilot() = default;

    void AddWaypoint(const Vector3 &position, float targetSpeed);

    [[nodiscard]] bool IsActive() const;

    Orientation CalculateSteering(const GameCamera &camera, float currentSpeed, float deltaTime);
};
