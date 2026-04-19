module;
#include "../lib/ray.hpp"
#include <entt/entt.hpp>

export module Prefabs:Player;

import JsonConfig;
import Components;
import RaylibResource;
import Types;
import Helpers;

export namespace factories {
entt::entity create_player(entt::registry &registry, const JsonConfig &config,
                           const Vector3 &start_position) {
  const auto player = registry.create();
  registry.emplace<Player>(player, QuaternionIdentity(),
                           start_position,  // position
                           Vector3Zero(),   // offset
                           Vector3Zero(),   // velocity
                           world_forward(), // forward
                           world_up(),      // up
                           world_right(),   // right
                           0.0f,            // speed
                           0.0f,
                           0.0f // effective ground height
  );
  registry.emplace<PlayerInputs>(player, 0.0f, 0.0f, 0.0f, 0.0f, true, true,
                                 false);
  registry.emplace<GroundHeight>(player, start_position.y, start_position.y);
  registry.emplace<Grounded>(player);

  // player is saved to the global context
  registry.ctx().emplace<PlayerEntity>(player);

  TraceLog(LOG_DEBUG, "player created");
  return player;
}
} // namespace factories
