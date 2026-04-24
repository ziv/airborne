module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module RenderSystem:ViewDebugging;

import Accessors;
import Components;

export void RenderDebugging(entt::registry &registry) {
  const auto view = registry.view<Position3D>(entt::exclude<World>);
  const auto &offset = get_offset(registry);

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
