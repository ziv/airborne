module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"
#include "rlgl.h"

export module RenderSystem:Models;

import Components;
import ResourceManager;
import Accessors;
import Resources;

export void RenderModels(entt::registry &registry) {
  const auto view = registry.view<Position3D, WithModel, Heading>(entt::exclude<World>);

  for (auto [entity, position, modeled, heading] : view.each()) {
    const auto models = get_resource_manager(registry).models;
    if (models.contains(modeled.model)) {
      const auto offset = registry.ctx().get<Offset>().offset;
      const auto p = position.pos + offset;
      DrawModelEx(models[modeled.model]->res, p, {0.0f, 1.0f, 0.0f}, heading.heading, {1.0f, 1.0f, 1.0f}, WHITE);
    }
  }
}

export namespace render_systems {

/// @brief render the sky model as background
void sky(entt::registry &registry) {
  const auto &player = get_player(registry);
  const auto models = get_resource_manager(registry).models;

  rlDisableDepthTest();
  rlDisableBackfaceCulling();
  DrawModel(models[resources::sky_model]->res, player.pos, 1.0f, WHITE);
  rlEnableBackfaceCulling();
  rlEnableDepthTest();
}
}  // namespace render_systems

// export void RenderModelsLabel(entt::registry &registry, const Camera3D &camera) {
//   const auto view = registry.view<Position3D, WithModel, Heading, Identity>(entt::exclude<World>);
//   const auto width = GetScreenWidth();
//   const auto height = GetScreenHeight();
//
//   for (auto [entity, position, modeled, heading, identify] : view.each()) {
//     const auto offset = registry.ctx().get<Offset>().offset;
//     const auto p = position.pos + offset;
//
//     const auto location = GetWorldToScreen(p, camera);
//     if (location.x < 0 || location.y < 0 || location.x > static_cast<float>(width) || location.y > static_cast<float>(height)) continue;
//
//     DrawText(identify.name.c_str(), static_cast<int>(location.x), static_cast<int>(location.y), 10, GREEN);
//   }
// }