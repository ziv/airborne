#pragma once
#include "GameData.h"
#include "raylib.h"

struct Waypoint {
    Vector3 Position;
    float TargetSpeed;
    float ArrivalRadius;
};

class Autopilot {
    std::vector<Waypoint> route;
    size_t currentWaypointIndex = 0;

    // limit the roll
    float maxBankAngle = 45.0f;

    // limit the pitch
    float maxPullRatio = 1.0f;

    // limit the throttle
    float speedRatio = 1.0f;

    bool active = false;

public:
    Autopilot(float maxBankAngle, float maxPullRatio, float speedRatio);

    void Toggle() { active = !active; }

    void AddWaypoint(const Vector3 &position, float targetSpeed, float arrivalRadius);

    [[nodiscard]] bool IsActive() const;

    PilotControls AutoSteer(GameData &game);
};
