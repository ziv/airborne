/**
 * @file Entity.h
 * @brief Base entity type and related enums for all in-game objects (aircraft,
 *        SAM sites, structures, airbases, etc.).
 */
#pragma once
#include "raylib.h"
#include "../primitives/Types.h"
#include <string>

/// Unique string identifier for an entity instance.
using EntityId = std::string;

/// @brief Alignment/team of an entity.
enum class Faction { FRIENDLY, ENEMY, NEUTRAL };

/// @brief Lifecycle state of an entity.
enum class EntityState {
    INACTIVE,    ///< Not yet activated.
    ACTIVE,      ///< Alive and operational.
    DAMAGED,     ///< Alive but below 50 % health.
    DESTROYED,   ///< Killed — remains in the registry for scoring/triggers.
    DESPAWNED    ///< Removed from the world entirely.
};

/// @brief Classification of an entity (determines behaviour and rendering).
enum class EntityType {
    AIRCRAFT,
    SAM,        ///< Surface-to-air missile site.
    AAA,        ///< Anti-aircraft artillery.
    STRUCTURE,  ///< Static buildings: bridges, depots, radars, bunkers.
    NAVAL,
    AIRBASE,    ///< Friendly or enemy airstrip / carrier.
    WAYPOINT    ///< Invisible navigation point.
};

/**
 * @brief Common data shared by every entity in the game world.
 *
 * Subclasses (e.g. GroundTarget) extend this with type-specific behaviour.
 */
struct EntityBase {
    EntityId id;
    EntityType type;
    std::string subtype;        ///< Optional sub-classification (e.g. "carrier" for AIRBASE).
    Faction faction = Faction::ENEMY;
    EntityState state = EntityState::ACTIVE;

    Vector3 position = {0, 0, 0};  ///< World-space position (before large-world offset).
    float heading = 0.0f;          ///< Compass heading in degrees (0 = north/+Z).
    float health = 100.0f;
    float maxHealth = 100.0f;

    std::string modelId;           ///< Path to the .glb model file, or empty for fallback cube.

    [[nodiscard]] bool isAlive() const {
        return state == EntityState::ACTIVE || state == EntityState::DAMAGED;
    }

    [[nodiscard]] bool isEnemy() const { return faction == Faction::ENEMY; }
};
