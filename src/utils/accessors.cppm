module;
#include <entt/entt.hpp>
#include "../lib/ray.hpp"

export module Accessors;

import Components;

constexpr auto PLAYER_ENTITY_CTX = entt::hashed_string("PlayerEntityID");
// registry.ctx().emplace<PlayerEntity>(player_entity);
// const auto player_entity = registry.ctx().get<entt::entity>(PLAYER_ENTITY_CTX);

export void set_player(entt::registry &registry, entt::entity player_entity) {
    registry.ctx().emplace_as<entt::entity>(PLAYER_ENTITY_CTX, player_entity);
}

export const entt::entity &get_player_entity(entt::registry &registry) {
    return registry.ctx().get<PlayerEntity>().id;
}

export const Player &get_player(entt::registry &registry) {
    return registry.get<Player>(get_player_entity(registry));
}

export const PlayerInputs &get_player_inputs(entt::registry &registry) {
    return registry.get<PlayerInputs>(get_player_entity(registry));
}

export Vector3 &get_offset(entt::registry &registry) {
    return registry.ctx().get<Offset>().offset;
}

export bool is_player_crashed(entt::registry &registry) {
    return registry.all_of<Crashed>(get_player_entity(registry));
}
