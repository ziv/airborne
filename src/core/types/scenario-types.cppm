module;
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "../../lib/ray.hpp"

export module Types:Scenario;

import :Units;

export {
  enum class Faction { FRIENDLY, ENEMY, NEUTRAL };

  NLOHMANN_JSON_SERIALIZE_ENUM(Faction, {{Faction::FRIENDLY, "friendly"}, {Faction::ENEMY, "enemy"}, {Faction::NEUTRAL, "neutral"}});

  enum class EntityState {
    INACTIVE,   ///< Not yet activated.
    ACTIVE,     ///< Alive and operational.
    DAMAGED,    ///< Alive but below 50 % health.
    DESTROYED,  ///< Killed — remains in the registry for scoring/triggers.
    DESPAWNED   ///< Removed from the world entirely.
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(EntityState, {{EntityState::INACTIVE, "inactive"},
                                             {EntityState::ACTIVE, "active"},
                                             {EntityState::DAMAGED, "damaged"},
                                             {EntityState::DESTROYED, "destroyed"},
                                             {EntityState::DESPAWNED, "despawned"}});

  enum class EntityType {
    None,
    AIRCRAFT,
    SAM,        ///< Surface-to-air missile site.
    AAA,        ///< Anti-aircraft artillery.
    STRUCTURE,  ///< Static buildings: bridges, depots, radars, bunkers.
    NAVAL,
    AIRBASE,  ///< Friendly or enemy airstrip / carrier.
    SHIP,
    CARRIER,  ///< Friendly or enemy airstrip / carrier.
    WAYPOINT  ///< Invisible navigation point.
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(EntityType, {{EntityType::None, "none"},
                                            {EntityType::AIRCRAFT, "aircraft"},
                                            {EntityType::SAM, "sam"},
                                            {EntityType::AAA, "aaa"},
                                            {EntityType::STRUCTURE, "structure"},
                                            {EntityType::NAVAL, "naval"},
                                            {EntityType::AIRBASE, "airbase"},
                                            {EntityType::SHIP, "ship"},
                                            {EntityType::CARRIER, "carrier"},
                                            {EntityType::WAYPOINT, "waypoint"}});

  enum class Difficulty { EASY, MEDIUM, HARD };

  NLOHMANN_JSON_SERIALIZE_ENUM(Difficulty, {{Difficulty::EASY, "easy"}, {Difficulty::MEDIUM, "medium"}, {Difficulty::HARD, "hard"}});

  enum class Weather { SUNNY, CLOUDY, OVERCAST, RAINY };

  NLOHMANN_JSON_SERIALIZE_ENUM(Weather, {{Weather::SUNNY, "sunny"}, {Weather::CLOUDY, "cloudy"}, {Weather::OVERCAST, "overcast"}, {Weather::RAINY, "rainy"}});

  enum class Season { SUMMER, WINTER };

  NLOHMANN_JSON_SERIALIZE_ENUM(Season, {{Season::SUMMER, "summer"}, {Season::WINTER, "winter"}});

  enum class TimeOfDay { DAY, NIGHT, DAWN, DUSK };

  NLOHMANN_JSON_SERIALIZE_ENUM(TimeOfDay, {{TimeOfDay::DAY, "day"}, {TimeOfDay::NIGHT, "night"}, {TimeOfDay::DAWN, "dawn"}, {TimeOfDay::DUSK, "dusk"}});

  enum class ObjectiveType {
    DESTROY,   ///< Eliminate a specific target entity.
    NAVIGATE,  ///< Reach a waypoint or area.
    ESCORT,    ///< Protect a friendly entity.
    SURVIVE    ///< Stay alive for a duration or until event.
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(ObjectiveType, {{ObjectiveType::DESTROY, "destroy"},
                                               {ObjectiveType::NAVIGATE, "navigate"},
                                               {ObjectiveType::ESCORT, "escort"},
                                               {ObjectiveType::SURVIVE, "survive"}});

  enum class WeaponCategory {
    AIR_TO_AIR,  ///< Air-to-air missiles (e.g. AIM-9, AIM-120).
    AGM,         ///< Air-to-ground missiles (e.g. AGM-65 Maverick).
    BOMB,        ///< Unguided/guided bombs (e.g. Mk-82, GBU-12).
    GUN,         ///< Aircraft cannon (e.g. M61 Vulcan).
    SAM,         ///< Surface-to-air missile system.
    AAA          ///< Anti-aircraft artillery system.
  };

  NLOHMANN_JSON_SERIALIZE_ENUM(WeaponCategory, {{WeaponCategory::AIR_TO_AIR, "air-to-air"},
                                                {WeaponCategory::AGM, "agm"},
                                                {WeaponCategory::BOMB, "bomb"},
                                                {WeaponCategory::GUN, "gun"},
                                                {WeaponCategory::SAM, "sam"},
                                                {WeaponCategory::AAA, "aaa"}});

  // for missiles only
  struct WeaponAgility {
    AngleDeg max_pitch = 0.0f;  ///< Maximum pitch rate (deg/s).
    AngleDeg max_yaw = 0.0f;    ///< Maximum yaw rate (deg/s).
    AngleDeg max_roll = 0.0f;   ///< Maximum roll rate (deg/s).
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(WeaponAgility, max_pitch, max_yaw, max_roll);

  struct WeaponEffects {
    Meter area_of_effect = 0.0f;      ///< Blast radius in meters.
    float armor_penetration = 0.0f;  ///< Armor penetration factor (0–100).
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(WeaponEffects, area_of_effect, armor_penetration);

  struct WeaponDef {
    std::string id;                                 ///< Unique weapon identifier (e.g. "aim-9m").
    std::string name;                               ///< Display name (e.g. "AIM-9M Sidewinder").
    WeaponCategory category = WeaponCategory::AGM;  ///< Weapon classification.

    Meter range = 0.0f;                 ///< Maximum engagement range in meters.
    float damage = 0.0f;                ///< Base damage per hit.
    int ammo_capacity = 1;               ///< Number of rounds/missiles carried.
    float rate_of_fire = 1.0f;            ///< Rounds per second (guns) or launches per second.
    MeterPerSecond speed = 0.0f;        ///< Initial projectile/missile speed.
    MeterPerSecond max_velocity = 0.0f;  ///< Maximum velocity after boost.

    WeaponAgility agility;  ///< Steering limits (guided missiles only).
    WeaponEffects effects;  ///< Damage area and penetration.

    bool player_selectable = false;  ///< Available in the player weapon selection screen.
    bool player_default = false;     ///< Pre-selected in the default loadout.
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(WeaponDef, id, name, category, range, damage, ammo_capacity, rate_of_fire, speed, max_velocity, agility, effects,
                                                  player_selectable, player_default);

  struct Waypoint {
    std::string name;
    Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Waypoint, name, position);

  struct EntityDef {
    std::string id;
    EntityType type = EntityType::None;
    std::string subtype;  ///< Optional subclassification (e.g. "carrier" for AIRBASE).
    Faction faction = Faction::ENEMY;
    EntityState state = EntityState::ACTIVE;

    Vector3 position = (Vector3){0, 0, 0};  ///< World-space position (before large-world offset).
    AngleDeg heading = 0.0f;                ///< Compass heading in degrees (0 = north/+Z).
    float health = 100.0f;
    float max_health = 100.0f;
    float scale = 1.0f;

    std::string model_id;  ///< Path to the .glb model file, or empty for fallback cube.

    /// Numeric behavior parameters keyed by name. Schema depends on entity
    /// type:
    /// - aircraft: speed, max_speed, altitude, fuelCapacity, fuelConsumption,
    /// turnRate, climbRate, radarRange
    /// - sam: detectionRange, engagementRange, maxAltitude, fireRate, accuracy,
    /// reloadTime
    /// - aaa: detectionRange, engagementRange, maxAltitude, fireRate, accuracy,
    /// burstDuration
    /// - airbase/carrier: minx, maxx, minz, maxz (landing zone area bounds)
    std::map<std::string, float> params = {};

    /// String properties keyed by name.
    std::map<std::string, std::string> properties = {};

    /// Weapon IDs assigned to this entity (references WeaponDef::id).
    std::vector<std::string> weapons = {};

    /// Ordered flight-path waypoints (aircraft entities only).
    std::vector<Waypoint> waypoints = {};

    [[nodiscard]] float get_param(const std::string &key, const float default_value = 0.0f) const {
      const auto it = params.find(key);
      return it != params.end() ? it->second : default_value;
    }

    [[nodiscard]] std::string get_property(const std::string &key, const std::string &default_value = "") const {
      const auto it = properties.find(key);
      return it != properties.end() ? it->second : default_value;
    }

    [[nodiscard]] bool is_alive() const { return state == EntityState::ACTIVE || state == EntityState::DAMAGED; }

    [[nodiscard]] bool is_enemy() const { return faction == Faction::ENEMY; }

    [[nodiscard]] bool is_friendly() const { return faction == Faction::FRIENDLY; }
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(EntityDef, id, type, subtype, faction, state, position, heading, health, max_health, model_id, params,
                                                  properties, weapons, waypoints);

  struct Objective {
    std::string id;                               ///< Unique objective identifier (e.g. "obj-01").
    ObjectiveType type = ObjectiveType::DESTROY;  ///< What the player must do.
    std::string target;                           ///< Target entity ID (for destroy/escort types).
    std::string label;                            ///< Short display text shown in the HUD.
    std::string description;                      ///< Longer description shown in briefing.
    int order = 0;                                ///< Display/priority order (lower = higher priority).
    bool optional = false;                        ///< If true, not required for mission success.
    bool hidden = false;                          ///< If true, revealed during gameplay rather than at briefing.
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Objective, id, type, target, label, description, order, optional, hidden);

  struct StartConditions {
    Vector3 position = {0.0f, 0.0f, 0.0f};
    float heading = 0.0f;
    MeterPerSecond speed = 0.0f;
    Meter altitude = 0.0f;
    float fuel = 3500.0f;  ///< Starting fuel in kilograms.
    bool carrier = false;  ///< True for a carrier catapult launch.

    /// Weapon IDs assigned to this player (references WeaponDef::id).
    std::vector<std::string> weapons = {};
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StartConditions, position, heading, speed, altitude, fuel, carrier, weapons);

  struct ResourceDef {
    std::string name{};
    std::string type{};
    std::string path{};
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ResourceDef, name, type, path)

  /*
  * "tex_path": "assets/tiles/north/z12-tex-bing/%d/%d.png",
    "hmp_path": "assets/tiles/north/z12-hmp-512/%d/%d.png",
    "meter_to_pixel": 8.05,
    "tex_size": 1024,
    "lowest": -735,
    "highest": 2814,
    "x_count": 17,
    "z_count": 15,
    "min_x": 2444,
    "min_z": 1644
   */
  struct TilesDef {
    std::string tex_path;
    std::string hmp_path;
    float tile_size_m = 0.0f;
    int base_x = 0;
    int base_z = 0;
    float meter_to_pixel = 0.0f;
    int tex_size = 0;
    float lowest = 0.0f;
    float highest = 0.0f;
    int x_count = 0;
    int z_count = 0;
    int min_x = 0;
    int min_z = 0;
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TilesDef, tex_path, hmp_path, tile_size_m, base_x, base_z, meter_to_pixel, tex_size, lowest, highest, x_count, z_count,
                                     min_x, min_z);

  struct Scenario {
    std::string id;
    std::string name;
    std::string description;
    Difficulty difficulty = Difficulty::EASY;
    Weather weather = Weather::SUNNY;
    Season season = Season::SUMMER;
    TimeOfDay time_of_day = TimeOfDay::DAY;
    Color sky_color{BLUE};
    std::string theater;  ///< Geographic region / map name.
    StartConditions start;

    std::vector<EntityDef> entities;    ///< Entities to spawn at mission start.
    std::vector<WeaponDef> weapons;     ///< Weapon type definitions used by entities.
    std::vector<Objective> objectives;  ///< Mission objectives (required + optional).
    std::vector<ResourceDef> resources;
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Scenario, id, name, description, difficulty, weather, season, time_of_day, sky_color, theater, start, entities,
                                                  weapons, objectives, resources);
}
