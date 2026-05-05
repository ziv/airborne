module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module RenderSystem:Cockpit;

import Components;
import ResourceManager;

export namespace render_systems_2d {
void cockpit(entt::registry &registry) {
  const auto &[texture, shader] = registry.ctx().get<CockpitWidget>();
  const auto &rm = get_resource_manager(registry);

  BeginShaderMode(rm.shaders[shader]->res);
  DrawTextureV(rm.textures[texture]->res, (Vector2){0.0f, 0.0f}, WHITE);
  EndShaderMode();
}
}  // namespace render_systems_2d
