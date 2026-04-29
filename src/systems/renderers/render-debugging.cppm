module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module RenderSystem:ViewDebugging;

import Accessors;
import Components;

export void RenderDebugging(entt::registry &registry) {
  const auto view = registry.view<Position3D>(entt::exclude<World>);
  const auto &offset = get_player(registry).offset;

  for (auto [entity, position] : view.each()) {
    const Vector3 drawPosition = position.pos + offset;
    DrawCubeWires(drawPosition, 150.0f, 150.0f, 150.0f, YELLOW);
    DrawCubeWires(drawPosition, 100.0f, 100.0f, 100.0f, YELLOW);
    DrawCubeWires(drawPosition, 50.0f, 50.0f, 50.0f, YELLOW);
    // DrawCube(drawPosition, 100.0f, 100.0f, 100.0f, YELLOW);
    // TraceLog(LOG_DEBUG, "Drawing debug cube  %s at position: (%f, %f)", id.name.c_str(), drawPosition.x, drawPosition.z);
    // TraceLog(LOG_DEBUG, "Drawing debug cube  %s at offset: (%f, %f)", id.name.c_str(), offset.x, offset.z);
  }
}

export namespace render_systems {

/// Add landing guides for carriers and airbase (TBI)
void render_debugging_landing(entt::registry &registry) {
  const auto &offset = get_player(registry).offset;
  for (const auto carrier_view = registry.view<Carrier, Position3D>(); auto [entity, position] : carrier_view.each()) {
    const auto pos = position.pos + offset;
    // touchdown area
    DrawCubeWires(pos, 200.0f, 150.0f, 500.0f, YELLOW);

    auto plane = pos + (Vector3){0.0f, 8.0f, 0.0f};
    DrawPlane(plane, (Vector2){200.0f, 500.0f}, Fade(YELLOW, 0.7f));

    for (int i = 0; i < 100; ++i) {
      auto p = plane + (Vector3){0.0f, 0.0f, 250.0f} + ((Vector3){0.0f, 0.0f, 20.0f} * i);
      DrawPlane(p, (Vector2){200.0f, 5.0f}, Fade(YELLOW, 0.5f));
    }

    for (int i = 0; i < 100; ++i) {
      auto p = plane + (Vector3){0.0f, 0.0f, -250.0f} + ((Vector3){0.0f, 0.0f, -20.0f} * i);
      DrawPlane(p, (Vector2){200.0f, 5.0f}, Fade(YELLOW, 0.5f));
    }
  }
}
}  // namespace render_systems