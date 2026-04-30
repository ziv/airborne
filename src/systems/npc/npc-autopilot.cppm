module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module Npc:Autopilot;

import Components;
import Accessors;

export namespace npc_systems {

void autopilot(entt::registry& registry, const float dt) {
  // const auto player_entity = get_player_entity(registry);
  const auto& player = get_player(registry);
  const Vector3 player_abs = player.absolute_position();

  const auto view = registry.view<NpcTag, AiController, AircraftOrientation, PhysicsProfile, Position3D, Velocity3D>();

  for (auto [entity, ai, orient, profile, pos, vel] : view.each()) {
    ai.fire_cooldown -= dt;

    const Vector3 npc_abs = pos.pos;  // NPCs have no large-world offset

    switch (ai.state) {
      case AiState::PATROL: {
        if (const auto* waypoints = registry.try_get<Waypoints>(entity); waypoints && !waypoints->waypoints.empty()) {
          const Vector3& wp = waypoints->waypoints[ai.waypoint_index];
          const Vector3 to_wp = wp - npc_abs;

          if (const float dist = Vector3Length(to_wp); dist < 500.0f) {
            TraceLog(LOG_DEBUG, "NPC reached waypoint %d (dist=%.0f m)", ai.waypoint_index, dist);
            ai.waypoint_index = (ai.waypoint_index + 1) % static_cast<int>(waypoints->waypoints.size());
          } else {
            ai.desired_dir = Vector3Normalize(to_wp);
          }
        }

        ai.desired_speed = profile.max_speed * 0.5f;

        // Detection check
        // const float player_dist = Vector3Distance(npc_abs, player_abs);
        // const auto* radar = registry.try_get<Radar>(entity);
        // if (radar && player_dist < radar->detection_range_m) {
        //   ai.state = AiState::ENGAGE;
        //   ai.target = player_entity;
        //   TraceLog(LOG_DEBUG, "NPC engage player (dist=%.0f m)", player_dist);
        // }
        break;
      }

      // ---- ENGAGE ----------------------------------------------------------
      case AiState::ENGAGE: {
        if (!registry.valid(ai.target)) {
          ai.state = AiState::PATROL;
          break;
        }

        const Vector3 to_player = player_abs - npc_abs;
        const float dist = Vector3Length(to_player);
        const auto* radar = registry.try_get<Radar>(entity);

        // Breakaway: too far, or no missiles left
        bool out_of_missiles = false;
        if (const auto* weapons = registry.try_get<Weapons>(entity)) {
          out_of_missiles = true;
          for (const auto& [name, count] : weapons->weapons) {
            if (count > 0) {
              out_of_missiles = false;
              break;
            }
          }
        }

        const float breakaway_range = radar ? radar->detection_range_m * 1.5f : 30000.0f;
        if (dist > breakaway_range || out_of_missiles) {
          ai.state = AiState::DISENGAGE;
          ai.disengage_timer = 10.0f;
          TraceLog(LOG_DEBUG, "NPC disengage (out_of_missiles=%d)", out_of_missiles ? 1 : 0);
          break;
        }

        // Steer toward player
        if (dist > 0.1f) ai.desired_dir = Vector3Normalize(to_player);

        ai.desired_speed = profile.max_speed;  // full throttle in combat

        // Fire check
        const float engagement_range = radar ? radar->engagement_range_m : 8000.0f;
        if (dist < engagement_range && ai.fire_cooldown <= 0.0f) {
          const float angle_cos = Vector3DotProduct(orient.forward, Vector3Normalize(to_player));
          if (angle_cos > 0.97f) {  // ~14 degree cone
            // Try to fire from weapon store
            if (auto* weapons = registry.try_get<Weapons>(entity)) {
              for (auto& [name, count] : weapons->weapons) {
                if (count > 0) {
                  --count;
                  const float rpm = registry.try_get<WeaponLauncher>(entity) ? static_cast<float>(registry.get<WeaponLauncher>(entity).fire_rate_rpm) : 20.0f;
                  ai.fire_cooldown = 60.0f / rpm;
                  TraceLog(LOG_DEBUG, "NPC fired %s (remaining=%d)", name.c_str(), count);
                  break;
                }
              }
            }
          }
        }
        break;
      }

      // ---- DISENGAGE -------------------------------------------------------
      case AiState::DISENGAGE: {
        ai.disengage_timer -= dt;

        const Vector3 to_player = player_abs - npc_abs;
        const float dist = Vector3Length(to_player);
        if (dist > 0.1f) ai.desired_dir = Vector3Normalize(Vector3Negate(to_player));  // fly away

        ai.desired_speed = profile.max_speed * 0.5f;  // back to cruise

        if (ai.disengage_timer <= 0.0f) {
          ai.state = AiState::PATROL;
          TraceLog(LOG_DEBUG, "NPC returning to patrol");
        }
        break;
      }
    }
  }
}

}  // namespace npc_systems
