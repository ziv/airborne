module;
#include <entt/entt.hpp>
#include "../lib/ray.hpp"
#include "rlgl.h"

export module WorldStreamerSystem;

import Components;
import Accessors;

export void WorldStreamerSystem(entt::registry &registry) {
    // is called streamer cause it original use was to stream
    // the world tiles

    // we are rendering the map at the offset of the player
    const auto &offset = get_offset(registry);
    const auto &inputs = get_player_inputs(registry);

    for (const auto view = registry.view<World, Position3D>(); auto [entity, world, pos]: view.each()) {

        // the "world"
        DrawModel(world.surface->res, offset, 1.0f, WHITE);

        // the "sky"
        // todo move this line to an update world system
        // world.clouds->res.materials[0].maps[MATERIAL_MAP_DIFFUSE].color.r += 0.01f * GetFrameTime();

        // todo find a better place to put it (above the player?!)
        const Vector3 cloudPosition = {
            offset.x,
            10000.0f,
            offset.y
        };
        rlDisableBackfaceCulling();
        DrawModel(world.clouds->res, cloudPosition, 1.0f, WHITE);
        rlEnableBackfaceCulling();

        // the "background"
        if (inputs.throttle > 0 && world.streams) {
            const float targetPitch = 0.8f + (inputs.throttle * 0.7f);
            const float targetVolume = 0.2f + (inputs.throttle * 0.9f);
            SetMusicPitch(world.streams->res, targetPitch);
            SetMusicVolume(world.streams->res, targetVolume);
            UpdateMusicStream(world.streams->res);
        }
    }
}
