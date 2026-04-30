module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module Npc:Physics;

import Components;
import Helpers;
import Utils;

export namespace npc_systems {

void physics(entt::registry& registry, const float dt) {
  const auto view = registry.view<NpcTag, AiController, AircraftOrientation, PhysicsProfile, Position3D, Velocity3D, Heading>();

  for (auto [entity, ai, orient, profile, pos, vel, heading] : view.each()) {

    // --- Rotation ---
    // Project desired_dir onto XZ (NPCs fly level).
    const Vector3 desired_xz = Vector3Normalize({ai.desired_dir.x, 0.0f, ai.desired_dir.z});
    const Vector3 current_xz = Vector3Normalize({orient.forward.x, 0.0f, orient.forward.z});

    // Angle between current heading and desired heading (signed, around Y).
    const float cos_a = Vector3DotProduct(current_xz, desired_xz);
    const float sin_a = Vector3DotProduct(Vector3CrossProduct(current_xz, desired_xz), world_up());
    const float angle = atan2f(sin_a, cos_a);  // signed angle in radians

    // Clamp rotation to max turn rate this frame.
    const float max_turn = profile.turn_rate * DEG2RAD * dt;
    const float step     = fmaxf(-max_turn, fminf(angle, max_turn));

    // Rotate orientation by step around world up.
    const Quaternion delta = QuaternionFromAxisAngle(world_up(), step);
    orient.rotation = QuaternionNormalize(QuaternionMultiply(delta, orient.rotation));

    // Rebuild axes from quaternion.
    orient.forward = Vector3RotateByQuaternion(world_forward(), orient.rotation);
    orient.up      = Vector3RotateByQuaternion(world_up(),      orient.rotation);
    orient.right   = Vector3RotateByQuaternion(world_right(),   orient.rotation);

    // --- Speed ---
    float speed = Vector3Length(vel.velocity);
    speed = utils::approach(speed, ai.desired_speed, 30.0f * dt);
    speed = fminf(speed, profile.max_speed);

    // --- Position ---
    vel.velocity    = orient.forward * speed;
    pos.pos         = pos.pos + vel.velocity * dt;
    heading.heading = atan2f(-orient.forward.x, orient.forward.z) * RAD2DEG;
  }
}

}  // namespace npc_systems