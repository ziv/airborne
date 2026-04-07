#pragma once
#include "raylib.h"
#include "../primitives/Types.h"
#include <string>

using EntityId = std::string;

enum class Faction { FRIENDLY, ENEMY, NEUTRAL };

enum class EntityState {
    INACTIVE,
    ACTIVE,
    DAMAGED,
    DESTROYED,
    DESPAWNED
};

enum class EntityType {
    AIRCRAFT,
    SAM,
    AAA,
    STRUCTURE,
    NAVAL,
    AIRBASE,
    WAYPOINT
};

struct EntityBase {
    EntityId id;
    EntityType type;
    std::string subtype;
    Faction faction = Faction::ENEMY;
    EntityState state = EntityState::ACTIVE;

    Vector3 position = {0, 0, 0};
    float heading = 0.0f;
    float health = 100.0f;
    float maxHealth = 100.0f;

    std::string modelId;

    [[nodiscard]] bool isAlive() const {
        return state == EntityState::ACTIVE || state == EntityState::DAMAGED;
    }

    [[nodiscard]] bool isEnemy() const { return faction == Faction::ENEMY; }
};
