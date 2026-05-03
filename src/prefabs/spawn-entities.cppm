module;
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include "../lib/ray.hpp"

export module Prefabs:Spawn;

import Types;
import Components;

void spawn_entity(entt::registry &registry, const nlohmann::json &entity) {
  if (!entity.contains("components")) {
    TraceLog(LOG_WARNING, "entity %s has no components", entity.dump().c_str());
    return;
  }

  if (!entity["components"].is_object()) {
    TraceLog(LOG_WARNING, "entity %s components is not an object", entity.dump().c_str());
    return;
  }
  const auto e = registry.create();

  for (const auto &[key, value] : entity["components"].items()) {
    if ("entity_id" == key) {
      const auto id = value["id"].get<std::string>();
      const auto name = value["name"].get<std::string>();
      registry.emplace<Identify>(e, id, name);
      TraceLog(LOG_DEBUG, "spawning component [%s]", id.c_str());
      continue;
    }

    TraceLog(LOG_DEBUG, "  > %s", key.c_str());

    if ("type" == key) {
      const auto type = value.get<EntityType>();
      registry.emplace<IdentifyType>(e, type);
      if (type == EntityType::CARRIER) {
        registry.emplace<Landable>(e, true);
        registry.emplace<Carrier>(e);
        registry.emplace<NpcTag>(e);
      } else if (type == EntityType::AIRBASE) {
        registry.emplace<Landable>(e, false);
      } else if (type == EntityType::AIRCRAFT) {
        registry.emplace<NpcTag>(e);
        registry.emplace<Velocity3D>(e, Vector3Zero());
      }
      continue;
    }

    if ("faction" == key) {
      const auto faction = value.get<Faction>();
      registry.emplace<FriendFoe>(e, faction);
      continue;
    }

    if ("position" == key) {
      const auto position = value.get<Vector3>();
      registry.emplace<Position3D>(e, position);
      continue;
    }

    if ("heading" == key) {
      const auto heading = value.get<float>();
      registry.emplace<Heading>(e, heading);
      continue;
    }

    if ("health" == key) {
      const auto max = value["max"].get<float>();
      registry.emplace<Health>(e, max, max);
      continue;
    }

    if ("identity" == key) {
      const auto name = value["name"].get<std::string>();
      const auto origin = value["origin"].get<std::string>();
      const auto type = value["type"].get<std::string>();
      registry.emplace<Identity>(e, name, origin, type);
      continue;
    }

    if ("physics_profile" == key) {
      const auto mass = value["mass_kg"].get<float>();
      const auto thrust = value["max_thrust_n"].get<float>();
      const auto lift = value["lift_coefficient"].get<float>();
      const auto drag = value["drag_coefficient"].get<float>();
      const auto max_speed = value["max_speed_m_s"].get<float>();
      const auto max_g = value["max_g_load"].get<float>();
      const auto turn_rate = value["turn_rate_deg_s"].get<float>();
      registry.emplace<PhysicsProfile>(e, mass, thrust, drag, lift, max_speed, max_g, turn_rate);
      continue;
    }

    if ("radar" == key) {
      const auto detection_range = value["detection_range_m"].get<float>();
      const auto engagement_range_m = value["engagement_range_m"].get<float>();
      registry.emplace<Radar>(e, detection_range, engagement_range_m);
      continue;
    }

    if ("prefab" == key) {
      // todo handle prefab
      continue;
    }

    // todo complete the data comes from weapon_launcher
    if ("weapon_launcher" == key) {
      const auto type = value["weapon_prefab_id"].get<std::string>();
      const auto fire_rate_rpm = value["fire_rate_rpm"].get<int>();
      const auto capacity = value["capacity"].get<int>();
      registry.emplace<WeaponLauncher>(e, type, fire_rate_rpm, capacity);
      continue;
    }

    if ("loadout" == key) {
      std::map<std::string, int> weapons;
      for (const auto &[weapon, amount] : value["weapons"].items()) {
        weapons[weapon] = amount.get<int>();
      }
      registry.emplace<Weapons>(e, weapons);
      continue;
    }

    if ("landing_zone" == key) {
      const auto runway_length_m = value["runway_length_m"].get<float>();
      const auto runway_width_m = value["runway_width_m"].get<float>();
      const auto elevation_m = value["elevation_m"].get<float>();

      // calculate landing zone
      // const float half_width = landable.carrier ? 100.0f : 200.0f;
      // const float halfLength = landable.carrier ? 250.0f : 2000.0f;
      // const float surface_y = landable.carrier ? 8.0f : 0.0f;

      registry.emplace<LandingZone>(e, runway_length_m, runway_width_m, elevation_m);
      continue;
    }

    if ("waypoints" == key) {
      std::vector<Vector3> waypoints;
      for (const auto &wp : value["points"]) {
        waypoints.push_back(wp.get<Vector3>());
      }
      registry.emplace<Waypoints>(e, waypoints);
      continue;
    }

    if ("model" == key) {
      const auto model_id = value.get<std::string>();
      const auto model = entt::hashed_string(value.get<std::string>().data());
      registry.emplace<WithModel>(e, model);
      continue;
    }

    TraceLog(LOG_ERROR, "unknown component %s", key.c_str());
  }
}

export namespace factories {

void spawn(entt::registry &registry, const nlohmann::json &entity) { spawn_entity(registry, entity); }
}  // namespace factories