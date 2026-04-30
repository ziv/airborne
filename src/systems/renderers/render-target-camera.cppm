module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module RenderSystem:TargetCamera;

import Components;

export namespace render_systems {
void target_camera(entt::registry &registry, Camera &camera) {
  // take the player current lock
  // put a 3d camera that follow it from the player position
  // render the target in a clipped screen (should we render the surrounding word?)

  const auto view = registry.view<DashboardSlot, TargetCameraWidget, Position2D>();
}
}  // namespace render_systems
