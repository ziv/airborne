module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"
#include "rlgl.h"

export module WorldStreamerSystem;

import Components;
import Accessors;
import TerrainStreaming;

export void WorldStreamerSystem(entt::registry &registry) {
  // is called streamer cause it original use was to stream
  // the world tiles

  // const auto view = registry.view<const TerrainChunk>();
  // for (const auto [entity, chunk] : view.each()) {
  //   // todo replace with resource manager
  //   const auto pos = (Vector3){chunk.x, chunk.y, chunk.z};
  //   // DrawModel(chunk.model, pos, 1.0f, WHITE);
  // }

  // we are rendering the map at the offset of the player
  const auto &offset = get_offset(registry);

  for (const auto view = registry.view<const World, const Position3D>(); auto [entity, world, pos] : view.each()) {
    // the "world"
    DrawModel(world.surface->res, offset, 1.0f, WHITE);

    // the "sky"
    // todo move this line to an update world system
    // world.clouds->res.materials[0].maps[MATERIAL_MAP_DIFFUSE].color.r +=
    // 0.01f * GetFrameTime();

    // todo find a better place to put it (above the player?!)
    // const Vector3 cloud_position = {offset.x, 10000.0f, offset.y};
    constexpr Vector3 cloud_position = {0.0f, 10000.0f, 0.0f};
    rlDisableBackfaceCulling();
    DrawModel(world.clouds->res, cloud_position, 1.0f, WHITE);
    rlEnableBackfaceCulling();
  }
}
