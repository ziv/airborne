module;
#include <entt/entt.hpp>
#include "../../lib/ray.hpp"

export module RenderSystem:ViewDebugging;

import Components;

export void RenderDebugging(entt::registry &registry) {
    auto view = registry.view<Position3D, Identify>(entt::exclude<World>);
    auto offset = registry.ctx().get<Offset>().offset;

    for (auto [entity, position, id]: view.each()) {
        const Vector3 drawPosition = position.pos + offset;
        DrawCubeWires(drawPosition, 100.0f, 100.0f, 100.0f, YELLOW);
        // DrawCube(drawPosition, 100.0f, 100.0f, 100.0f, YELLOW);
        // TraceLog(LOG_DEBUG, "Drawing debug cube  %s at position: (%f, %f)", id.name.c_str(), drawPosition.x, drawPosition.z);
        // TraceLog(LOG_DEBUG, "Drawing debug cube  %s at offset: (%f, %f)", id.name.c_str(), offset.x, offset.z);
    }
}
