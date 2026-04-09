/**
 * @file Trigger.h
 * @brief Event-driven scripting system — conditions, actions, and one-shot/repeatable triggers.
 *
 * Triggers fire when their condition is met (zone entry, entity destroyed,
 * time elapsed, etc.) and execute an action (spawn entity, radio message,
 * play sound, etc.). A trigger marked `once = true` fires at most one time.
 */
#pragma once
#include "../entities/Entity.h"
#include <string>

/// @brief What event must occur for a trigger to fire.
enum class TriggerType {
    ZONE_ENTER,
    ZONE_EXIT,
    ENTITY_DESTROYED,
    ENTITY_DETECTS_PLAYER,
    TIME_ELAPSED,
    OBJECTIVE_COMPLETE,
    ALTITUDE_BELOW,
    ALTITUDE_ABOVE,
    SPEED_BELOW,
    DAMAGE_TAKEN
};

/// @brief What happens when a trigger fires.
enum class TriggerActionType {
    SPAWN_ENTITY,
    DESTROY_ENTITY,
    RADIO_MESSAGE,
    UPDATE_OBJECTIVE,
    PLAY_SOUND,
    SET_WEATHER,
    ACTIVATE_TRIGGER
};

/// @brief Parameters that define when a trigger's condition is satisfied.
struct TriggerCondition {
    TriggerType type;
    EntityId entityId;
    Vector3 zoneCenter = {0, 0, 0};
    float radius = 0;
    float value = 0;
    std::string objectiveId;
};

/// @brief Payload describing what happens when a trigger fires.
struct TriggerAction {
    TriggerActionType type;
    std::string message;
    EntityId entityId;
    std::string objectiveId;
    std::string soundFile;
};

/// @brief A condition→action pair that fires during gameplay.
struct Trigger {
    std::string id;
    TriggerCondition condition;
    TriggerAction action;
    bool once = true;    ///< When true, the trigger fires at most once.
    bool fired = false;  ///< Set to true after a one-shot trigger has fired.
};
