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
  static PlayerPhysicsConfig conf = get_config(registry).player.aircraft;

  auto& player = get_player(registry);
  const auto& inputs = get_player_inputs(registry);

  // angular velocity

  // how much angular velocity we have in relation to the speed (more speed -> better steering)
  auto speed_ratio = player.speed / (conf.maxSpeed * 0.8f);
  if (speed_ratio < 0.0f) speed_ratio = 0.0f;
  if (speed_ratio > 1.2f) speed_ratio = 1.2f;
  float control_authority = speed_ratio * speed_ratio;

  if (control_authority < conf.minAuthority) control_authority = conf.minAuthority;

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
  const Vector3 yaw_torque = Vector3Scale(player.up, (inputs.yaw * conf.yawRatio * control_authority + induced_yaw) * dt);

  player.angular_velocity = player.angular_velocity + pitch_torque + roll_torque + yaw_torque;

  // (2.5 is some air dumping factor)
  float damping_factor = 1.0f - (conf.airDumpingFactor * dt);
  if (damping_factor < 0.0f) damping_factor = 0.0f;
  player.angular_velocity *= damping_factor;

  // linear velocity

  const auto square_speed = player.speed * player.speed;
  const float mass = conf.weight / 9.81f;

  auto cd = conf.dragCoefficient;
  if (inputs.brakes) cd += cd * 3.0f;
  if (inputs.gear) cd += cd * 3.0f;

  const auto drag = square_speed * cd;
  const auto thrust = inputs.throttle * conf.engineThrust;
  const auto lift = square_speed * conf.liftCoefficient;

  // ground forces
  if (!is_player_flying(registry)) {
    auto normal_face = conf.weight - lift;
    if (normal_face < 0.0f) {
      normal_face = 0.0f;
    }
    const float friction_coefficient = inputs.brakes ? 0.6f : 0.02f;
    const float max_braking_force = friction_coefficient * normal_face;

    const Vector3 ground_velocity = {player.velocity.x, 0.0f, player.velocity.z};
    float ground_speed = Vector3Length(ground_velocity);

    if (ground_speed > 0.1f) {
      const Vector3 brakingDirection = ground_velocity * (-1.0f / ground_speed);
      const float brakingAccelerationMag = max_braking_force / mass;
      const Vector3 brakingAcceleration = brakingDirection * brakingAccelerationMag;

      player.velocity = player.velocity + (brakingAcceleration * dt);

      // don't go back...
      const Vector3 new_ground_velocity = {player.velocity.x, 0.0f, player.velocity.z};
      if (Vector3DotProduct(ground_velocity, new_ground_velocity) < 0.0f) {
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

  // --- Force vectors (world space) ---
  const auto thrustForce = player.forward * thrust;
  const auto dragForce = player.forward * -drag;
  const auto liftForce = player.up * lift;
  const auto weightForce = gravity() * mass;

  // todo on ground, add friction force
  // todo on ground, add brakes friction force
  // conf.frictionCoefficient

  const auto total = thrustForce + dragForce + weightForce + liftForce;
  const auto acceleration = total * 1 / mass;

  // todo  keeping forces globally only for debug view, remove later
  registry.ctx().insert_or_assign(Forces{thrust, drag, lift, mass, acceleration});

  // --- Euler integration ---
  player.velocity = player.velocity + (acceleration * dt);
  player.speed = Vector3Length(player.velocity);

  // hard speed cap (normally drag balances thrust before this limit)
  if (player.speed > conf.maxSpeed && player.speed != 0.0f) {
    player.velocity = player.velocity * conf.maxSpeed / player.speed;
    player.speed = Vector3Length(player.velocity);
  }

  // don't mess with near zero speed
  if (player.speed < 0.01f) {
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
}
}  // namespace player_systems
