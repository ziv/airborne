/**
 * @file Entity.h
 * @brief Base entity type and related enums for all in-game objects (aircraft,
 *        SAM sites, structures, airbases, etc.).
 */
#pragma once
#include "raylib.h"
#include "../lib/json.h"
#include <string>
#include "../core/AircraftStructs.h"

/// Unique string identifier for an entity instance.
using EntityId = std::string;

/// @brief Alignment/team of an entity.
enum class Faction { FRIENDLY, ENEMY, NEUTRAL };

NLOHMANN_JSON_SERIALIZE_ENUM(Faction, {
                             {Faction::FRIENDLY, "friendly"},
                             {Faction::ENEMY, "enemy"},
                             {Faction::NEUTRAL, "neutral"}
                             });

/// @brief Lifecycle state of an entity.
enum class EntityState {
    INACTIVE, ///< Not yet activated.
    ACTIVE, ///< Alive and operational.
    DAMAGED, ///< Alive but below 50 % health.
    DESTROYED, ///< Killed — remains in the registry for scoring/triggers.
    DESPAWNED ///< Removed from the world entirely.
};

NLOHMANN_JSON_SERIALIZE_ENUM(EntityState, {
                             {EntityState::INACTIVE, "inactive"},
                             {EntityState::ACTIVE, "active"},
                             {EntityState::DAMAGED, "damaged"},
                             {EntityState::DESTROYED, "destroyed"},
                             {EntityState::DESPAWNED, "despawned"}
                             });

/// @brief Classification of an entity (determines behavior and rendering).
enum class EntityType {
    None,
    AIRCRAFT,
    SAM, ///< Surface-to-air missile site.
    AAA, ///< Anti-aircraft artillery.
    STRUCTURE, ///< Static buildings: bridges, depots, radars, bunkers.
    NAVAL,
    AIRBASE, ///< Friendly or enemy airstrip / carrier.
    CARRIER, ///< Friendly or enemy airstrip / carrier.
    WAYPOINT ///< Invisible navigation point.
};

NLOHMANN_JSON_SERIALIZE_ENUM(EntityType, {
                             {EntityType::AIRCRAFT, "aircraft"},
                             {EntityType::SAM, "sam"},
                             {EntityType::AAA, "aaa"},
                             {EntityType::STRUCTURE, "structure"},
                             {EntityType::NAVAL, "naval"},
                             {EntityType::AIRBASE, "airbase"},
                             {EntityType::CARRIER, "carrier"},
                             {EntityType::WAYPOINT, "waypoint"}
                             });

/**
 * @brief Common data shared by every entity in the game world.
 *
 * Subclasses (e.g. GroundTarget) extend this with type-specific behavior.
 */
struct EntityDef {
    EntityId id = "";
    EntityType type = EntityType::None;
    std::string subtype = ""; ///< Optional subclassification (e.g. "carrier" for AIRBASE).
    Faction faction = Faction::ENEMY;
    EntityState state = EntityState::ACTIVE;

    Vector3 position = {0, 0, 0}; ///< World-space position (before large-world offset).
    float heading = 0.0f; ///< Compass heading in degrees (0 = north/+Z).
    float health = 100.0f;
    float maxHealth = 100.0f;
    float scale = 1.0f;

    std::string modelId = ""; ///< Path to the .glb model file, or empty for fallback cube.

    std::map<std::string, float> params = {};

    std::map<std::string, std::string> properties = {};

    float getParam(const std::string &key, const float defaultValue = 0.0f) const {
        const auto it = params.find(key);
        return it != params.end() ? it->second : defaultValue;
    }

    std::string getProperty(const std::string &key, const std::string &defaultValue = "") const {
        const auto it = properties.find(key);
        return it != properties.end() ? it->second : defaultValue;
    }

    [[nodiscard]] bool isAlive() const {
        return state == EntityState::ACTIVE || state == EntityState::DAMAGED;
    }

    [[nodiscard]] bool isEnemy() const { return faction == Faction::ENEMY; }

    [[nodiscard]] bool isFriendly() const { return faction == Faction::FRIENDLY; }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(EntityDef, id, type, subtype, faction, state, position, heading, health, maxHealth, modelId, params);

class EntityBase {
protected:
    EntityDef def;
    Color color = GRAY;

public:
    explicit EntityBase(EntityDef _et) : def(std::move(_et)) {
        if (isEnemy()) color = RED;
        if (isFriendly()) color = GREEN;
    }

    virtual ~EntityBase() = default;

    /// @brief Get the current world-space position of the entity, applying the large-world offset.
    Vector3 position(const AircraftState &state) const;

    virtual void draw(const AircraftState &state);

    virtual void update(AircraftState &state, float dt) {
        // default: no per-frame behavior
    }

    [[nodiscard]] bool isAlive() const {
        return def.state == EntityState::ACTIVE || def.state == EntityState::DAMAGED;
    }

    [[nodiscard]] bool isEnemy() const { return def.faction == Faction::ENEMY; }

    [[nodiscard]] bool isFriendly() const { return def.faction == Faction::FRIENDLY; }
};
