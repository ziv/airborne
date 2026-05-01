module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module PlayerSystems:Physics;

import Components;
import Accessors;
import Types;
import Helpers;

export namespace player_systems {

void physics(entt::registry& registry, const float dt) {
  const PlayerPhysicsConfig conf = get_config(registry).player.aircraft;

  auto& player = get_player(registry);
  const auto& inputs = get_player_inputs(registry);
  const auto flying = is_player_flying(registry);

  // angular velocity

  // how much angular velocity we have in relation to the speed (more speed -> better steering)
  auto speed_ratio = std::clamp(player.speed / (conf.maxSpeed * 0.8f), 0.0f, 1.2f);
  float control_authority = speed_ratio * speed_ratio;

  // on ground, we allow user to yaw
  float yaw_authority = flying ? control_authority : conf.minAuthority;

  // some effects...

  // slip, bank angle induces a yaw moment toward the lowered wing.
  const float induced_yaw = player.right.y * conf.bankInduceYawRatio;

  // lift-loss pitch-down: when the lift vector is no longer purely vertical the
  // aircraft tends to pitch nose-down.
  const float lift_loss = 1.0f - player.up.y;
  const float induced_pitch = -lift_loss * conf.liftLossPitchRatio;

  // all together
  const Vector3 pitch_torque = Vector3Scale(player.right, (inputs.pitch * conf.pitchRatio * control_authority + induced_pitch) * dt);
  const Vector3 roll_torque = Vector3Scale(player.forward, inputs.roll * conf.rollRatio * control_authority * dt);
  const Vector3 yaw_torque = Vector3Scale(player.up, (inputs.yaw * conf.yawRatio * yaw_authority + induced_yaw) * dt);

  player.angular_velocity = player.angular_velocity + pitch_torque + roll_torque + yaw_torque;

  // some air dumping
  float damping_factor = 1.0f - conf.airDumpingFactor * dt;
  if (damping_factor < 0.0f) damping_factor = 0.0f;
  player.angular_velocity *= damping_factor;

  // linear velocity

  const auto square_speed = player.speed * player.speed;
  const float mass = conf.weight / 9.81f;

  Ratio cd = conf.dragCoefficient;
  if (inputs.brakes) cd += cd * 3.0f;
  if (inputs.gear) cd += cd * 3.0f;

  const float drag = square_speed * cd;
  const float thrust = inputs.throttle * conf.engineThrust;
  const float lift = square_speed * conf.liftCoefficient;

  // ground forces
  if (!flying) {
    auto normal_face = conf.weight - lift;
    if (normal_face < 0.0f) {
      normal_face = 0.0f;
    }

    // todo should be very high for carrier (0.9 for example)
    const float friction_coefficient = inputs.brakes ? 0.6f : 0.02f;
    const float max_braking_force = friction_coefficient * normal_face;

    const Vector3 ground_velocity = {player.velocity.x, 0.0f, player.velocity.z};

    if (float ground_speed = Vector3Length(ground_velocity); ground_speed > 0.1f) {
      const Vector3 braking_direction = ground_velocity * (-1.0f / ground_speed);
      const float braking_acceleration_mag = max_braking_force / mass;
      const Vector3 braking_acceleration = braking_direction * braking_acceleration_mag;

      player.velocity = player.velocity + braking_acceleration * dt;

      // don't go back...
      if (const Vector3 new_ground_velocity = {player.velocity.x, 0.0f, player.velocity.z}; Vector3DotProduct(ground_velocity, new_ground_velocity) < 0.0f) {
        player.velocity.x = 0.0f;
        player.velocity.z = 0.0f;
      }
    } else {
      if (inputs.brakes) {
        player.velocity.x = 0.0f;
        player.velocity.z = 0.0f;
      }
    }
  }

  // force vectors
  const auto thrust_force = player.forward * thrust;
  const auto drag_force = player.forward * -drag;
  const auto lift_force = player.up * lift;
  const auto weight_force = gravity() * mass;

  const auto total = thrust_force + drag_force + weight_force + lift_force;
  const auto acceleration = total * 1 / mass;

  player.velocity = player.velocity + acceleration * dt;
  player.speed = Vector3Length(player.velocity);

  // hard speed cap (normally drag balances thrust before this limit)
  if (player.speed > conf.maxSpeed && player.speed != 0.0f) {
    player.velocity = player.velocity * conf.maxSpeed / player.speed;
    player.speed = conf.maxSpeed;
  }

  // don't mess with near zero speed
  if (player.speed < 0.02f) {
    player.velocity = Vector3Zero();
    player.speed = 0.0f;
  }

  // weathervaning: align velocity toward the nose above stall speed
  // Artificial Aerodynamic Stability/Velocity Alignment
  // https://en.wikipedia.org/wiki/Weathervane_effect
  if (player.speed > conf.stallSpeed) {
    const Vector3 target_velocity = player.forward * player.speed;
    const float alignment_speed = 1.0f + (player.speed * 0.05f);
    player.velocity = Vector3Lerp(player.velocity, target_velocity, alignment_speed * dt);
  }

  // calculate G
  // const auto properAccel = acceleration - gravity();
  // const Vector3 localAccel = Vector3RotateByQuaternion(properAccel, QuaternionInvert(player.rotation));
  // const float gForceVertical = localAccel.y / 9.81f;      // head/ass
  // float gForceLateral = localAccel.x / 9.81f;       // right/left
  // float gForceLongitudinal = localAccel.z / 9.81f;  // front/back
}
}  // namespace player_systems
