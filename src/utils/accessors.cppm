module;
#include <entt/entt.hpp>

export module Accessors;

import AppOptions;
import Components;
import ResourceManager;
import JsonConfig;
import Types;

export entt::registry initial_registry(const AppConfig &config, const AppOptions &opts) {
  entt::registry registry;
  registry.ctx().emplace<GameState>(GameStatus::INITIALIZE);
  registry.ctx().emplace<Configuration>(config);
  registry.ctx().emplace<AppOptions>(opts);
  create_resource_manager(registry);
  return registry;
}

export AppOptions &get_options(entt::registry &registry) { return registry.ctx().get<AppOptions>(); }

export const AppConfig &get_config(entt::registry &registry) { return registry.ctx().get<Configuration>().conf; }

export const entt::entity &get_player_entity(entt::registry &registry) { return registry.ctx().get<PlayerEntity>().id; }

export Player &get_player(entt::registry &registry) { return registry.get<Player>(get_player_entity(registry)); }

export PlayerInputs &get_player_inputs(entt::registry &registry) { return registry.get<PlayerInputs>(get_player_entity(registry)); }

export bool is_player_crashed(entt::registry &registry) { return registry.all_of<Crashed>(get_player_entity(registry)); }

export bool is_player_flying(entt::registry &registry) { return registry.all_of<Flying>(get_player_entity(registry)); }

export bool is_player_grounded(entt::registry &registry) { return registry.all_of<Grounded>(get_player_entity(registry)); }

export LandingZoneDef &get_landing_zone(entt::registry &registry) { return registry.get<LandingZoneDef>(get_player_entity(registry)); }