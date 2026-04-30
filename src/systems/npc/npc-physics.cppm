module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module Npc:Physics;

import Components;
import :Steering;

export namespace npc_systems {

void physics(entt::registry& registry, const float dt) {
  const auto view = registry.view<NpcTag, AiController, AircraftOrientation,
                                   PhysicsProfile, Position3D, Velocity3D, Heading>();

  for (auto [entity, ai, orient, profile, pos, vel, heading] : view.each()) {
    // --- Rotate toward desired_dir -------------------------------------------
    const float max_turn_rad = profile.turn_rate * DEG2RAD * dt;

    // Build the target quaternion from desired_dir and world up.
    const Vector3 desired_norm =
      Vector3LengthSqr(ai.desired_dir) > 0.0001f ? Vector3Normalize(ai.desired_dir) : orient.forward;

    // Compute target rotation: rotate world_forward to desired_norm in XZ,
    // then tilt pitch as needed.  We reconstruct from axes directly.
    const Vector3 new_fwd_xz = {desired_norm.x, desired_norm.y, desired_norm.z};
    const Vector3 world_up   = {0.0f, 1.0f, 0.0f};
    const Vector3 new_right  = Vector3Normalize(Vector3CrossProduct(new_fwd_xz, world_up));
    const Vector3 new_up     = Vector3CrossProduct(new_right, new_fwd_xz);

    // Pack into a target quaternion via the rotation matrix axes.
    // raylib: QuaternionFromMatrix works with Matrix directly.
    const Matrix m = {
      new_right.x, new_up.x, -new_fwd_xz.x, 0.0f,
      new_right.y, new_up.y, -new_fwd_xz.y, 0.0f,
      new_right.z, new_up.z, -new_fwd_xz.z, 0.0f,
      0.0f,        0.0f,      0.0f,          1.0f
    };
    const Quaternion target_rot = QuaternionFromMatrix(m);

    // Slerp by at most max_turn_rad toward the target orientation.
    const float dot = orient.rotation.x * target_rot.x + orient.rotation.y * target_rot.y
                    + orient.rotation.z * target_rot.z + orient.rotation.w * target_rot.w;
    const float angle = 2.0f * acosf(fminf(fabsf(dot), 1.0f));
    const float t_slerp = (angle > 0.0001f) ? fminf(max_turn_rad / angle, 1.0f) : 1.0f;
    orient.rotation = QuaternionSlerp(orient.rotation, target_rot, t_slerp);
    orient.rotation = QuaternionNormalize(orient.rotation);

    // Rebuild forward/up/right from quaternion.
    orient.forward = Vector3RotateByQuaternion({0.0f, 0.0f, 1.0f}, orient.rotation);
    orient.up      = Vector3RotateByQuaternion({0.0f, 1.0f, 0.0f}, orient.rotation);
    orient.right   = Vector3RotateByQuaternion({1.0f, 0.0f, 0.0f}, orient.rotation);

    // --- Speed / velocity ---------------------------------------------------
    float speed = Vector3Length(vel.velocity);

    // Approach desired speed.
    const float speed_delta = 30.0f * dt;  // ~30 m/s²
    if (speed < ai.desired_speed - speed_delta) speed += speed_delta;
    else if (speed > ai.desired_speed + speed_delta) speed -= speed_delta;
    else speed = ai.desired_speed;

    speed = fminf(speed, profile.max_speed);

    // Weathervane: align velocity to nose.
    const Vector3 target_vel = orient.forward * speed;
    vel.velocity = Vector3Lerp(vel.velocity, target_vel, 3.0f * dt);

    // --- Position integration -----------------------------------------------
    pos.pos = pos.pos + vel.velocity * dt;

    // --- Keep Heading in sync (for renderer + radar) ------------------------
    heading.heading = atan2f(orient.forward.x, orient.forward.z) * RAD2DEG;
  }
}

}  // namespace npc_systems
