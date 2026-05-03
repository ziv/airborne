module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module AircraftSystems:UpdateLock;

import Accessors;
import Components;
import Types;

export namespace aircraft_systems {
void update_lock(entt::registry& registry) {
  const auto player = get_player(registry);
  auto& radar_state = registry.get<RadarState>(get_player_entity(registry));

  bool search = false;

  if (IsKeyPressed(KEY_M)) {
    radar_state.mode = radar_state.mode == RadarMode::AIR_TO_AIR ? RadarMode::AIR_TO_GROUND : RadarMode::AIR_TO_AIR;
    search = true;
  }

  const auto player_abs_position = player.pos - player.offset;

  // check the current lock...
  if (radar_state.locked_target != entt::null) {
    // is it still in valid entity
    if (registry.valid(radar_state.locked_target)) {
      const auto& pos = registry.get<Position3D>(radar_state.locked_target).pos;

      // is it still in range?!
      if (const auto distance = Vector3Distance(player_abs_position, pos); distance > radar_state.max_range) {
        radar_state.locked_target = entt::null;
      }
    } else {
      // not relevant anymore...
      radar_state.locked_target = entt::null;
    }
  }

  // if "T" pressed or mode changed, update target
  if (IsKeyPressed(KEY_T) || search) {
    TraceLog(LOG_WARNING, "updating lock");

    // "browse" the targets
    std::vector<entt::entity> valid_targets;

    // collect all valid targets in range
    for (const auto view = registry.view<const IdentifyType, const Position3D>(); auto entity : view) {
      const auto& type_comp = view.get<const IdentifyType>(entity).type;
      const auto& target_pos = view.get<const Position3D>(entity).pos;

      bool is_valid_type = false;

      if (radar_state.mode == RadarMode::AIR_TO_AIR) {
        is_valid_type = type_comp == EntityType::AIRCRAFT;
      }
      if (radar_state.mode == RadarMode::AIR_TO_GROUND) {
        if (type_comp == EntityType::AAA || type_comp == EntityType::SAM || type_comp == EntityType::STRUCTURE || type_comp == EntityType::NAVAL ||
            type_comp == EntityType::AIRBASE || type_comp == EntityType::SHIP || type_comp == EntityType::CARRIER) {
          is_valid_type = true;
        }
      }

      if (is_valid_type) {
        if (Vector3Distance(player_abs_position, target_pos) <= radar_state.max_range) {
          valid_targets.push_back(entity);
        }
      }
    }

    TraceLog(LOG_DEBUG, "found %d entities", valid_targets.size());

    if (valid_targets.empty()) {
      // no targets in range
      radar_state.locked_target = entt::null;
      return;
    }

    // sort to keep the browsing in the same order
    std::ranges::sort(valid_targets, [&](const entt::entity a, const entt::entity b) {
      const float distA = Vector3Distance(player_abs_position, registry.get<Position3D>(a).pos);
      const float distB = Vector3Distance(player_abs_position, registry.get<Position3D>(b).pos);
      return distA < distB;
    });

    if (auto it = std::ranges::find(valid_targets, radar_state.locked_target); it != valid_targets.end()) {
      // we found the current. move next
      ++it;
      if (it == valid_targets.end()) {
        // end of the vector, start over
        radar_state.locked_target = valid_targets.front();
      } else {
        // found and set
        radar_state.locked_target = *it;
      }
    } else {
      radar_state.locked_target = valid_targets.front();
    }
  }
}
}  // namespace aircraft_systems