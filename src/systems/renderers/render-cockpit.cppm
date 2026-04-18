module;
#include <entt/entt.hpp>
#include "../../lib/ray.hpp"

export module RenderSystem:Cockpit;

import Components;

export void RenderCockpit(entt::registry &registry) {
    const auto view = registry.view<CockpitWidget, WithFsShader, WithTexture, Position2D>();
    if (view.begin() == view.end()) return;

    const entt::entity entity = view.front();
    const auto [fs, tx, pos] = registry.get<WithFsShader, WithTexture, Position2D>(entity);

    BeginShaderMode(fs.handle->res);
    DrawTextureV(tx.handle->res, pos.pos, WHITE);
    EndShaderMode();
}
