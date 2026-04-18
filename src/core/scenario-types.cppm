module;
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <map>
#include "../lib/ray.hpp"

export module Types:Scenario;

import :Units;

export
{
    enum class Faction { FRIENDLY, ENEMY, NEUTRAL };

    NLOHMANN_JSON_SERIALIZE_ENUM(Faction, {
                                 {Faction::FRIENDLY, "friendly"},
                                 {Faction::ENEMY, "enemy"},
                                 {Faction::NEUTRAL, "neutral"}
                                 });

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

    struct EntityDef {
        std::string id;
        EntityType type = EntityType::None;
        std::string subtype; ///< Optional subclassification (e.g. "carrier" for AIRBASE).
        Faction faction = Faction::ENEMY;
        EntityState state = EntityState::ACTIVE;

        Vector3 position = (Vector3){0, 0, 0}; ///< World-space position (before large-world offset).
        float heading = 0.0f; ///< Compass heading in degrees (0 = north/+Z).
        float health = 100.0f;
        float maxHealth = 100.0f;
        float scale = 1.0f;

        std::string modelId; ///< Path to the .glb model file, or empty for fallback cube.

        std::map<std::string, float> params = {};

        std::map<std::string, std::string> properties = {};

        [[nodiscard]] float getParam(const std::string &key, const float defaultValue = 0.0f) const {
            const auto it = params.find(key);
            return it != params.end() ? it->second : defaultValue;
        }

        [[nodiscard]] std::string getProperty(const std::string &key, const std::string &defaultValue = "") const {
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

    struct StartConditions {
        Vector3 position = {0.0f, 0.0f, 0.0f};
        float heading = 0.0f;
        MeterPerSecond speed = 0.0f;
        Meter altitude = 0.0f;
        float fuel = 3500.0f; ///< Starting fuel in kilograms.
        bool carrier = false; ///< True for a carrier catapult launch.
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StartConditions, position, heading, speed, altitude, fuel, carrier);

    struct Scenario {
        std::string id;
        std::string name;
        std::string description;
        std::string difficulty;
        std::string theater; ///< Geographic region / map name.

        StartConditions start;
        // WeaponLoadout loadout;

        std::vector<EntityDef> entities; ///< Entities to spawn at mission start.
        // std::vector<Objective> objectives; ///< Mission objectives (required + optional).
        // std::vector<Trigger> triggers; ///< Event-driven scripting triggers.
        // ScenarioScoring scoring;
        Color skyColor{BLUE};
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Scenario, id, name, description, difficulty, theater, start, entities, skyColor);

}
