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

  if (IsKeyPressed(KEY_M)) {
    radar_state.mode = radar_state.mode == RadarMode::AIR_TO_AIR ? RadarMode::AIR_TO_GROUND : RadarMode::AIR_TO_AIR;
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

  // if "T" pressed, update target
  if (!IsKeyPressed(KEY_T)) return;

  TraceLog(LOG_WARNING, "T pressed, updating lock");

  // "browse" the targets
  std::vector<entt::entity> valid_targets;

  // collect all valid targets in range
  for (auto view = registry.view<IdentifyType, Position3D>(); auto entity : view) {
    const auto& type_comp = view.get<IdentifyType>(entity).type;
    const auto& target_pos = view.get<Position3D>(entity).pos;

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
      float dist = Vector3Distance(player_abs_position, target_pos);
      TraceLog(LOG_DEBUG, "checking entity %d of type %d - valid %d, distance %f", static_cast<int>(entity), static_cast<int>(type_comp), is_valid_type, dist);
      if (dist <= radar_state.max_range) {
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
  std::ranges::sort(valid_targets, [&](entt::entity a, entt::entity b) {
    const float distA = Vector3Distance(player_abs_position, registry.get<Position3D>(a).pos);
    const float distB = Vector3Distance(player_abs_position, registry.get<Position3D>(b).pos);
    return distA < distB;
  });

  // todo what if locked target is null?
  auto it = std::ranges::find(valid_targets, radar_state.locked_target);

  if (it != valid_targets.end()) {
    // we found the current. move next
    TraceLog(LOG_WARNING, "we found the current. move next");
    ++it;
    if (it == valid_targets.end()) {
      // end of the vector, start over
      TraceLog(LOG_WARNING, "end of the vector, start over");
      radar_state.locked_target = valid_targets.front();
    } else {
      TraceLog(LOG_WARNING, "found and set");
      radar_state.locked_target = *it;
    }
  } else {
    TraceLog(LOG_WARNING, "point to the start");
    radar_state.locked_target = valid_targets.front();
  }
}
}  // namespace aircraft_systems