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

NLOHMANN_JSON_SERIALIZE_ENUM(TriggerType, {
                             {TriggerType::ZONE_ENTER, "zone_enter"},
                             {TriggerType::ZONE_EXIT, "zone_exit"},
                             {TriggerType::ENTITY_DESTROYED, "entity_destroyed"},
                             {TriggerType::ENTITY_DETECTS_PLAYER, "entity_detects_player"},
                             {TriggerType::TIME_ELAPSED, "time_elapsed"},
                             {TriggerType::OBJECTIVE_COMPLETE, "objective_complete"},
                             {TriggerType::ALTITUDE_BELOW, "altitude_below"},
                             {TriggerType::ALTITUDE_ABOVE, "altitude_above"},
                             {TriggerType::SPEED_BELOW, "speed_below"},
                             {TriggerType::DAMAGE_TAKEN, "damage_taken"}
                             });

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

NLOHMANN_JSON_SERIALIZE_ENUM(TriggerActionType, {
                             {TriggerActionType::SPAWN_ENTITY, "spawn_entity"},
                             {TriggerActionType::DESTROY_ENTITY, "destroy_entity"},
                             {TriggerActionType::RADIO_MESSAGE, "radio_message"},
                             {TriggerActionType::UPDATE_OBJECTIVE, "update_objective"},
                             {TriggerActionType::PLAY_SOUND, "play_sound"},
                             {TriggerActionType::SET_WEATHER, "set_weather"},
                             {TriggerActionType::ACTIVATE_TRIGGER, "activate_trigger"}
                             });

/// @brief Parameters that define when a trigger's condition is satisfied.
struct TriggerCondition {
    TriggerType type;
    EntityId entityId;
    Vector3 zoneCenter = {0, 0, 0};
    float radius = 0;
    float value = 0;
    std::string objectiveId;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TriggerCondition, type, entityId, zoneCenter, radius, value, objectiveId);

/// @brief Payload describing what happens when a trigger fires.
struct TriggerAction {
    TriggerActionType type;
    std::string message;
    EntityId entityId;
    std::string objectiveId;
    std::string soundFile;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TriggerAction, type, message, entityId, objectiveId, soundFile);

/// @brief A condition→action pair that fires during gameplay.
struct Trigger {
    std::string id;
    TriggerCondition condition;
    TriggerAction action;
    bool once = true; ///< When true, the trigger fires at most once.
    bool fired = false; ///< Set to true after a one-shot trigger has fired.
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Trigger, id, condition, action, once, fired);