/**
 * @file Objective.h
 * @brief Mission objective definitions — types, statuses, and completion parameters.
 */
#pragma once
#include "../entities/Entity.h"
#include "../primitives/Types.h"
#include <string>
#include  "../lib/json.h"

/// @brief The kind of action the player must perform to complete an objective.
enum class ObjectiveType {
    DESTROY, ///< Destroy a specific target entity.
    PHOTOGRAPH, ///< Fly over a target at the correct angle/altitude.
    NAVIGATE, ///< Reach a waypoint within a given radius.
    INTERCEPT, ///< Engage and shoot down an airborne target.
    ESCORT, ///< Protect a friendly entity en route to a destination.
    SEAD, ///< Suppress/destroy enemy air defenses.
    SURVIVE ///< Stay alive for a specified duration.
};

NLOHMANN_JSON_SERIALIZE_ENUM(ObjectiveType, {
                             {ObjectiveType::DESTROY, "destroy"},
                             {ObjectiveType::PHOTOGRAPH, "photograph"},
                             {ObjectiveType::NAVIGATE, "navigate"},
                             {ObjectiveType::INTERCEPT, "intercept"},
                             {ObjectiveType::ESCORT, "escort"},
                             {ObjectiveType::SEAD, "sead"},
                             {ObjectiveType::SURVIVE, "survive"}
                             });

/// @brief Lifecycle state of a single objective.
enum class ObjectiveStatus {
    PENDING, ///< Not yet started.
    ACTIVE, ///< Currently in progress.
    COMPLETED, ///< Successfully finished.
    FAILED ///< Failed (target lost, time expired, etc.).
};

NLOHMANN_JSON_SERIALIZE_ENUM(ObjectiveStatus, {
                             {ObjectiveStatus::PENDING, "pending"},
                             {ObjectiveStatus::ACTIVE, "active"},
                             {ObjectiveStatus::COMPLETED, "completed"},
                             {ObjectiveStatus::FAILED, "failed"}
                             });

/// @brief Optional constraints that refine objective completion criteria.
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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ObjectiveParams, minAltitude, maxAltitude, maxDistance, requiredAngle, arrivalRadius,
                                   durationSeconds, escortTarget, escortDestination, zoneCenter, zoneRadius);

/// @brief A single mission objective shown on the briefing screen and tracked in-game.
struct Objective {
    std::string id;
    ObjectiveType type;
    std::string label; ///< Human-readable description shown in the briefing.
    EntityId targetEntityId; ///< Entity this objective relates to (if any).
    bool required = true; ///< Required objectives must be completed to win.
    int displayOrder = 0; ///< Sort order on the briefing screen.
    ObjectiveStatus status = ObjectiveStatus::PENDING;
    ObjectiveParams params;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Objective, id, type, label, targetEntityId, required, displayOrder, status, params);
