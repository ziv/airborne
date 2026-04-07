#pragma once
#include "../entities/Entity.h"
#include <string>

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

enum class TriggerActionType {
    SPAWN_ENTITY,
    DESTROY_ENTITY,
    RADIO_MESSAGE,
    UPDATE_OBJECTIVE,
    PLAY_SOUND,
    SET_WEATHER,
    ACTIVATE_TRIGGER
};

struct TriggerCondition {
    TriggerType type;
    EntityId entityId;
    Vector3 zoneCenter = {0, 0, 0};
    float radius = 0;
    float value = 0;
    std::string objectiveId;
};

struct TriggerAction {
    TriggerActionType type;
    std::string message;
    EntityId entityId;
    std::string objectiveId;
    std::string soundFile;
};

struct Trigger {
    std::string id;
    TriggerCondition condition;
    TriggerAction action;
    bool once = true;
    bool fired = false;
};
