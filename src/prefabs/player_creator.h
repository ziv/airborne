#pragma once
#include "../lib/json.h"
#include <entt/entt.hpp>
#include "../components/world.h"
#include "../primitives/AppConfig.h"

struct PilotConfig {
    float fov;
    float tilt;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PilotConfig, fov, tilt);

namespace Factory {
    inline entt::entity createPlayer(entt::registry &registry,
                                     const AppConfig &config) {
        const auto pilotConfig = config.get<PilotConfig>("/pilot");
        const auto player = registry.create();

        registry.emplace<Player>(player);
        registry.emplace<View3D>(player, pilotConfig.fov, pilotConfig.tilt);

        return player;
    }
}
