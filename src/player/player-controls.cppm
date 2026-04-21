module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module Player:Controls;

import Components;
import :Config;

float approach(float current, const float target, const float maxDelta) {
  if (current < target) {
    current += maxDelta;
    if (current > target) current = target;
  } else if (current > target) {
    current -= maxDelta;
    if (current < target) current = target;
  }
  return current;
}

export class PlayerControls {
  PlayerControlsConfig conf;

 public:
  explicit PlayerControls(const PlayerControlsConfig &c) : conf(c) {}

  void update(entt::registry &registry, const float dt) const {
    // this will steer the aircraft as long as there is no autopilot set
    for (const auto view = registry.view<Player, PlayerInputs>(entt::exclude<Autopilot>); auto [entity, player, inputs] : view.each()) {
      float inputSpeed = 2.0f;
      float step = inputSpeed * dt;

      float targetRoll = 0.0f;
      if (IsKeyDown(KEY_RIGHT)) targetRoll = 1.0f;
      if (IsKeyDown(KEY_LEFT)) targetRoll = -1.0f;
      inputs.roll = approach(inputs.roll, targetRoll, step);
      // inputs.roll = Lerp(inputs.roll, targetRoll, 2.0f * dt);

      float targetPitch = 0.0f;
      if (IsKeyDown(KEY_UP)) targetPitch = -1.0f;
      if (IsKeyDown(KEY_DOWN)) targetPitch = 1.0f;
      inputs.pitch = approach(inputs.pitch, targetPitch, step);
      // inputs.pitch = Lerp(inputs.pitch, targetPitch, 2.0f * dt);

      float targetYaw = 0.0f;
      if (IsKeyDown(KEY_Q)) targetYaw = 1.0f;
      if (IsKeyDown(KEY_E)) targetYaw = -1.0f;
      inputs.yaw = approach(inputs.yaw, targetYaw, step);

      // inputs.pitch = 0.0f;
      // inputs.roll = 0.0f;
      // inputs.yaw = 0.0f;
      //
      // if (IsKeyDown(KEY_UP)) inputs.pitch = -conf.pitchRatio * dt;
      // if (IsKeyDown(KEY_DOWN)) inputs.pitch = conf.pitchRatio * dt;
      //
      // if (IsKeyDown(KEY_LEFT)) inputs.roll = -conf.rollRatio * dt;
      // if (IsKeyDown(KEY_RIGHT)) inputs.roll = conf.rollRatio * dt;
      //
      // if (IsKeyDown(KEY_Q)) inputs.yaw = conf.yawRatio * dt;
      // if (IsKeyDown(KEY_E)) inputs.yaw = -conf.yawRatio * dt;

      if (IsKeyPressed(KEY_B)) inputs.brakes = !inputs.brakes;
    }
  }
};
