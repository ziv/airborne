module;
#include "../lib/ray.hpp"
#include <entt/entt.hpp>

export module Player:Physics;

import Types;
import Helpers;
import Components;
import :Config;

export class PlayerPhysics {
  PlayerPhysicsConfig conf;

public:
  explicit PlayerPhysics(const PlayerPhysicsConfig &c) : conf(c) {}

  void update(entt::registry &registry, const float dt) const {
    for (const auto view = registry.view<Player, PlayerInputs>();
         auto [entity, player, inputs] : view.each()) {
      const auto squareSpeed = player.speed * player.speed;

      auto cd = conf.dragCoefficient;
      if (inputs.brakes)
        cd += cd * 3.0f;
      if (inputs.gear)
        cd += cd * 3.0f;

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
      registry.ctx().insert_or_assign(
          Forces{thrust, drag, lift, mass, acceleration});

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
        player.velocity =
            Vector3Lerp(player.velocity, target_velocity, alignment_speed * dt);
      }
    }
  }
};
