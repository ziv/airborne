module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module Accessors;

import Components;
import ResourceManager;
import JsonConfig;
import Types;

export void set_initial_globals(
  entt::registry &registry,
  const AppConfig &config,
  const JsonConfig &scenario
  ) {
  registry.ctx().emplace<GameState>(GameStatus::INITIALIZE);
  registry.ctx().emplace<Configuration>(config, scenario);
  create_resource_manager(registry);
}

export const AppConfig &get_config(entt::registry &registry) { return registry.ctx().get<Configuration>().conf; }

export const entt::entity &get_player_entity(entt::registry &registry) { return registry.ctx().get<PlayerEntity>().id; }

export Player &get_player(entt::registry &registry) { return registry.get<Player>(get_player_entity(registry)); }

export PlayerInputs &get_player_inputs(entt::registry &registry) { return registry.get<PlayerInputs>(get_player_entity(registry)); }

// export Vector3 &get_offset(entt::registry &registry) { return registry.ctx().get<Offset>().offset; }

export bool is_player_crashed(entt::registry &registry) { return registry.all_of<Crashed>(get_player_entity(registry)); }

export bool is_player_flying(entt::registry &registry) { return registry.all_of<Flying>(get_player_entity(registry)); }