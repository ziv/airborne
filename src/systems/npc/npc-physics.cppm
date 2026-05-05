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
    // yaw (rotation around world up)
    // Project desired_dir onto XZ for the heading component.
    const Vector3 desired_xz = Vector3Normalize({ai.desired_dir.x, 0.0f, ai.desired_dir.z});
    const Vector3 current_xz = Vector3Normalize({orient.forward.x, 0.0f, orient.forward.z});

    // signed angle between current heading and desired heading (around Y).
    const float cos_a = Vector3DotProduct(current_xz, desired_xz);
    const float sin_a = Vector3DotProduct(Vector3CrossProduct(current_xz, desired_xz), world_up());
    const float angle = atan2f(sin_a, cos_a);

    // clamp yaw to max turn rate this frame.
    const float max_turn = profile.turn_rate * DEG2RAD * dt;
    const float yaw_step = fmaxf(-max_turn, fminf(angle, max_turn));

    const Quaternion yaw_delta = QuaternionFromAxisAngle(world_up(), yaw_step);
    orient.rotation = QuaternionNormalize(QuaternionMultiply(yaw_delta, orient.rotation));

    // rebuild axes after yaw so pitch uses the updated right vector.
    orient.forward = Vector3RotateByQuaternion(world_forward(), orient.rotation);
    orient.up = Vector3RotateByQuaternion(world_up(), orient.rotation);
    orient.right = Vector3RotateByQuaternion(world_right(), orient.rotation);

    // pitch (climb/dive toward desired vertical component) ---
    // desired_dir is a unit vector, so its Y component is sin(pitch).
    constexpr float max_pitch_rad = 80.0f * DEG2RAD;
    const float desired_dir_y = fmaxf(-1.0f, fminf(1.0f, ai.desired_dir.y));
    float desired_pitch = asinf(desired_dir_y);
    desired_pitch = fmaxf(-max_pitch_rad, fminf(desired_pitch, max_pitch_rad));

    const float current_pitch = asinf(fmaxf(-1.0f, fminf(1.0f, orient.forward.y)));
    const float pitch_diff = desired_pitch - current_pitch;

    // reuse turn_rate for pitch rate
    const float max_pitch_step = profile.turn_rate * DEG2RAD * dt;
    const float pitch_step = fmaxf(-max_pitch_step, fminf(pitch_diff, max_pitch_step));

    // positive rotation around `right` rotates forward toward up (nose up).
    const Quaternion pitch_delta = QuaternionFromAxisAngle(orient.right, pitch_step);
    orient.rotation = QuaternionNormalize(QuaternionMultiply(pitch_delta, orient.rotation));

    // rebuild axes from final quaternion.
    orient.forward = Vector3RotateByQuaternion(world_forward(), orient.rotation);
    orient.up = Vector3RotateByQuaternion(world_up(), orient.rotation);
    orient.right = Vector3RotateByQuaternion(world_right(), orient.rotation);

    // speed and position
    float speed = Vector3Length(vel.velocity);
    speed = utils::approach(speed, ai.desired_speed, 30.0f * dt);
    speed = fminf(speed, profile.max_speed);
    vel.velocity = orient.forward * speed;
    pos.pos = pos.pos + vel.velocity * dt;
    heading.heading = atan2f(-orient.forward.x, orient.forward.z) * RAD2DEG;
  }
}
}  // namespace npc_systems