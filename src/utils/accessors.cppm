module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module Accessors;

import Components;
import ResourceManager;
import Types;

// constexpr auto PLAYER_ENTITY_CTX = entt::hashed_string("PlayerEntityID");
// registry.ctx().emplace<PlayerEntity>(player_entity);
// return registry.ctx().get<PlayerEntity>().id;
// const auto player_entity = registry.ctx().get<entt::entity>(PLAYER_ENTITY_CTX);

// export void set_player(entt::registry &registry, entt::entity player_entity) {
// registry.ctx().emplace_as<entt::entity>(PLAYER_ENTITY_CTX, player_entity);
// }

export void set_initial_globals(entt::registry &registry, const AppConfig &config, const Scenario &scenario) {
  registry.ctx().emplace<GameState>(GameStatus::INITIALIZE);
  registry.ctx().emplace<Offset>(Vector3Zero());
  registry.ctx().emplace<Configuration>(config, scenario);
  create_resource_manager(registry);
}

export const AppConfig &get_config(entt::registry &registry) { return registry.ctx().get<Configuration>().conf; }

export const Scenario &get_scenario(entt::registry &registry) { return registry.ctx().get<Configuration>().scenario; }

export const entt::entity &get_player_entity(entt::registry &registry) { return registry.ctx().get<PlayerEntity>().id; }

export Player &get_player(entt::registry &registry) { return registry.get<Player>(get_player_entity(registry)); }

export PlayerInputs &get_player_inputs(entt::registry &registry) { return registry.get<PlayerInputs>(get_player_entity(registry)); }

export Vector3 &get_offset(entt::registry &registry) { return registry.ctx().get<Offset>().offset; }

export bool is_player_crashed(entt::registry &registry) { return registry.all_of<Crashed>(get_player_entity(registry)); }

export bool is_player_flying(entt::registry &registry) { return registry.all_of<Flying>(get_player_entity(registry)); }