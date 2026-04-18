module;
#include <entt/entt.hpp>
#include "../lib/ray.hpp"

export module Prefabs:Player;

import JsonConfig;
import Components;
import RaylibResource;
import Types;
import Helpers;

export namespace factories {
    entt::entity create_player(entt::registry &registry,
                              const JsonConfig &config,
                              const Scenario &scenario) {
        const auto player = registry.create();
        registry.emplace<Player>(player,
                                 QuaternionIdentity(),
                                 scenario.start.position, // position
                                 Vector3Zero(), // offset
                                 Vector3Zero(), // velocity
                                 world_forward(), // forward
                                 world_up(), // up
                                 world_right(), // right
                                 0.0f // speed
        );
        registry.emplace<PlayerInputs>(player, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false);
        registry.emplace<GroundHeight>(player, scenario.start.position.y, scenario.start.position.y);
        registry.emplace<Grounded>(player);

        // player is saved to the global context
        registry.ctx().emplace<PlayerEntity>(player);
        return player;
    }
}
