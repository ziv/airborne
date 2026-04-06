#pragma once
#include <vector>
#include "../core/AircraftStructs.h"
#include "../primitives/Resource.h"
#include "../primitives/Types.h"

enum AircraftMode {
    Patrol,
    Engage,
    Flee
};

struct AircraftWaypoint {
    Vector3 position;
    float speed;
    float arrivalRadius;
};

struct AircraftSteer {
    float roll;
    float pitch;
    float throttle;
};

class Aircraft {
private:
    // limit the roll
    float maxBankAngle = 45.0f;

    // limit the pitch
    float maxPullRatio = 1.0f;

    // limit the throttle
    float speedRatio = 1.0f;
    float maxTurnRate = 0.5f;


    MeterPerSecond speed = 10.0;
    Vector3 forward = {-1.0f, 0.0f, 0.0f};

    // ModelHandle model;

    std::pmr::vector<AircraftWaypoint> waypoints;
    size_t currentWaypointIndex = 0;

    void steer(float dt);

public:
    Vector3 position = {500.0f, 1000.0f, 500.0f};
    Aircraft();

    void update(const AircraftState &state, float dt);

    void draw();
};
