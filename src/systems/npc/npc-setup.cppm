module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module Npc:Setup;

import Components;

export namespace npc_systems {

void setup(entt::registry& registry) {
  const auto view = registry.view<NpcTag, PhysicsProfile, Heading, Velocity3D>(entt::exclude<AiController>);

  for (const auto [entity, profile, heading, vel] : view.each()) {
    const float hdg_rad = heading.heading * DEG2RAD;
    const Vector3 fwd = {sinf(hdg_rad), 0.0f, cosf(hdg_rad)};

    const Quaternion rot = QuaternionFromEuler(0.0f, -hdg_rad, 0.0f);
    const Vector3 up    = {0.0f, 1.0f, 0.0f};
    const Vector3 right = Vector3CrossProduct(fwd, up);

    registry.emplace_or_replace<AircraftOrientation>(entity, rot, fwd, up, right);

    const float cruise = profile.max_speed * 0.5f;
    vel.velocity = fwd * cruise;

    registry.emplace<AiController>(entity,
      AiState::PATROL,
      0,
      entt::entity{entt::null},
      0.0f,
      0.0f,
      fwd,
      cruise);
  }
}

}  // namespace npc_systems
