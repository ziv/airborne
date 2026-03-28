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
    float arrivalRadius = 20.0f;
    bool active = false;

public:
    Autopilot() = default;

    void Toggle() { this->active = !this->active; }

    void AddWaypoint(const Vector3 &position, float targetSpeed);

    [[nodiscard]] bool IsActive() const;

    Orientation CalculateSteering(const Vector3 &camPos,
                                  const Vector3 &forward,
                                  const Vector3 &up,
                                  const Vector3 &right,
                                  float currentSpeed,
                                  float deltaTime);
};
