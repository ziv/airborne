#pragma once
#include <entt/entt.hpp>
#include "../components/world.h"
#include "../primitives/AppConfig.h"

namespace Factory {
    inline entt::entity createPlayer(entt::registry &registry,
                                     const AppConfig &config) {
        const auto player = registry.create();

        registry.emplace<Player>(player);
        registry.emplace<View3D>(player, config.get<float>("/pilot/fov"), config.get<float>("/pilot/tilt"));

        return player;
    }
}
