module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module RenderSystem:Cockpit;

import Components;
import ResourceManager;

export void render_cockpit(entt::registry &registry) {
  const auto view = registry.view<CockpitWidget, WithFsShader, WithTexture, Position2D>();
  if (view.begin() == view.end()) return;

  const entt::entity entity = view.front();
  auto [fs, tx, pos] = registry.get<const WithFsShader, const WithTexture, const Position2D>(entity);

  const auto &rm = get_resource_manager(registry);

  BeginShaderMode(rm.shaders[fs.shader]->res);
  DrawTextureV(rm.textures[tx.texture]->res, pos.pos, WHITE);
  EndShaderMode();
}
