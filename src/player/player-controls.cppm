module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module Player:Controls;

import Components;
import :Config;

export class PlayerControls {
  PlayerControlsConfig conf;

 public:
  explicit PlayerControls(const PlayerControlsConfig &c) : conf(c) {}

  void update(entt::registry &registry, const float dt) const {
    // this will steer the aircraft as long as there is no autopilot set
    for (const auto view = registry.view<Player, PlayerInputs>(entt::exclude<Autopilot>); auto [entity, player, inputs] : view.each()) {
      inputs.pitch = 0.0f;
      inputs.roll = 0.0f;
      inputs.yaw = 0.0f;

      if (IsKeyDown(KEY_UP)) inputs.pitch = -conf.pitchRatio * dt;
      if (IsKeyDown(KEY_DOWN)) inputs.pitch = conf.pitchRatio * dt;

      if (IsKeyDown(KEY_LEFT)) inputs.roll = -conf.rollRatio * dt;
      if (IsKeyDown(KEY_RIGHT)) inputs.roll = conf.rollRatio * dt;

      if (IsKeyDown(KEY_Q)) inputs.yaw = conf.yawRatio * dt;
      if (IsKeyDown(KEY_E)) inputs.yaw = -conf.yawRatio * dt;

      if (IsKeyPressed(KEY_B)) inputs.brakes = !inputs.brakes;
    }
  }
};
