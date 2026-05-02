module;
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

export module Accessors;

import Components;
import ResourceManager;
import JsonConfig;
import Types;

export void set_initial_globals(entt::registry &registry, const AppConfig &config, const nlohmann::json &options) {
  registry.ctx().emplace<GameState>(GameStatus::INITIALIZE);
  registry.ctx().emplace<GameOptions>(false, options["tilt"].get<float>(), options["fov"].get<float>(), options["tiles_token"].get<std::string>(),
                                      options["maps_token"].get<std::string>());
  registry.ctx().emplace<Configuration>(config);
  create_resource_manager(registry);
}

export GameOptions &get_options(entt::registry &registry) { return registry.ctx().get<GameOptions>(); }

export const AppConfig &get_config(entt::registry &registry) { return registry.ctx().get<Configuration>().conf; }

export const entt::entity &get_player_entity(entt::registry &registry) { return registry.ctx().get<PlayerEntity>().id; }

export Player &get_player(entt::registry &registry) { return registry.get<Player>(get_player_entity(registry)); }

export PlayerInputs &get_player_inputs(entt::registry &registry) { return registry.get<PlayerInputs>(get_player_entity(registry)); }

export bool is_player_crashed(entt::registry &registry) { return registry.all_of<Crashed>(get_player_entity(registry)); }

export bool is_player_flying(entt::registry &registry) { return registry.all_of<Flying>(get_player_entity(registry)); }