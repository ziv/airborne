module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module Player:Physics;

import Types;
import Helpers;
import Components;
import Accessors;
import :Config;

// angular velocity
constexpr RadSecSquared PITCH_POWER = 3.0f;
constexpr RadSecSquared ROLL_POWER = 5.0f;
constexpr RadSecSquared YAW_POWER = 1.0f;
constexpr Ratio SKIP_YAW_FACTOR = 0.6f;
constexpr Ratio PITCH_DOWN_FACTOR = 1.1f;
constexpr Ratio AIR_DUMPING_FACTOR = 2.5f;
constexpr Ratio MIN_AUTHORITY = 0.5f;

export class PlayerPhysics {
  PlayerPhysicsConfig conf;

 public:
  explicit PlayerPhysics(const PlayerPhysicsConfig& c) : conf(c) {}

  void update(entt::registry& registry, const float dt) const {
    auto& player = get_player(registry);
    const auto& inputs = get_player_inputs(registry);

    // angular velocity
    // todo angular velocity size should be relative to the aircraft speed, more speed -> better steering
    auto speed_ratio = player.speed / (conf.maxSpeed * 0.7f);
    if (speed_ratio < 0.0f) speed_ratio = 0.0f;
    if (speed_ratio > 1.2f) speed_ratio = 1.2f;
    float control_authority = speed_ratio * speed_ratio;

    if (control_authority < MIN_AUTHORITY) control_authority = MIN_AUTHORITY;

    // some effects...

    // slip, bank angle induces a yaw moment toward the lowered wing.
    const float induced_yaw = player.right.y * SKIP_YAW_FACTOR;

    // lift-loss pitch-down: when the lift vector is no longer purely vertical the
    // aircraft tends to pitch nose-down.
    const float lift_loss = 1.0f - player.up.y;
    const float induced_pitch = -lift_loss * PITCH_DOWN_FACTOR;

    const Vector3 pitch_torque = Vector3Scale(player.right, (inputs.pitch * PITCH_POWER * control_authority + induced_pitch) * dt);
    const Vector3 roll_torque = Vector3Scale(player.forward, inputs.roll * ROLL_POWER * control_authority * dt);
    const Vector3 yaw_torque = Vector3Scale(player.up, (inputs.yaw * YAW_POWER * control_authority + induced_yaw) * dt);

    player.angular_velocity = player.angular_velocity + pitch_torque + roll_torque + yaw_torque;

    // (2.5 is some air dumping factor)
    float damping_factor = 1.0f - (AIR_DUMPING_FACTOR * dt);
    if (damping_factor < 0.0f) damping_factor = 0.0f;
    player.angular_velocity *= damping_factor;

    // linear velocity
    const auto squareSpeed = player.speed * player.speed;

    auto cd = conf.dragCoefficient;
    if (inputs.brakes) cd += cd * 3.0f;
    if (inputs.gear) cd += cd * 3.0f;

    auto drag = squareSpeed * cd;
    const auto thrust = inputs.throttle * conf.engineThrust;
    const auto lift = squareSpeed * conf.liftCoefficient;

    // on ground there is a constant friction
    // todo complete this add drag
    // if (!registry.all_of<Grounded>(entity)) {
    //   drag *= 10.0;
    // }

    // todo complete me
    // there is no such thing a negative drag
    // if (drag > thrust)
    //   drag = thrust;

    const float mass = conf.weight / 9.81f;

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
    if (player.speed < 0.18f) {
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
};
