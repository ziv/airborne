#pragma once
#include "../entities/Entity.h"
#include "../primitives/Types.h"
#include <string>

enum class ObjectiveType {
    DESTROY,
    PHOTOGRAPH,
    NAVIGATE,
    INTERCEPT,
    ESCORT,
    SEAD,
    SURVIVE
};

enum class ObjectiveStatus {
    PENDING,
    ACTIVE,
    COMPLETED,
    FAILED
};

struct ObjectiveParams {
    Meter minAltitude = 0;
    Meter maxAltitude = 99999;
    Meter maxDistance = 500;
    Degree requiredAngle = 30;
    Meter arrivalRadius = 2000;
    float durationSeconds = 0;
    EntityId escortTarget;
    EntityId escortDestination;
    Vector3 zoneCenter = {0, 0, 0};
    Meter zoneRadius = 0;
};

struct Objective {
    std::string id;
    ObjectiveType type;
    std::string label;
    EntityId targetEntityId;
    bool required = true;
    int displayOrder = 0;
    ObjectiveStatus status = ObjectiveStatus::PENDING;
    ObjectiveParams params;
};
