module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module PlayerSystems:Controls;

import Components;
import Types;
import Utils;

export namespace player_systems {

void controls(entt::registry& registry, const float dt) {
  // this will steer the aircraft as long as there is no autopilot set
  for (const auto view = registry.view<Player, PlayerInputs>(entt::exclude<Autopilot>); auto [entity, player, inputs] : view.each()) {
    constexpr auto stick_max = 3.0f;
    float target_roll = 0.0f;
    if (IsKeyDown(KEY_RIGHT)) target_roll = 1.0f;
    if (IsKeyDown(KEY_LEFT)) target_roll = -1.0f;
    inputs.roll = utils::approach(inputs.roll, target_roll, stick_max * dt);

    float target_pitch = 0.0f;
    if (IsKeyDown(KEY_UP)) target_pitch = -1.0f;
    if (IsKeyDown(KEY_DOWN)) target_pitch = 1.0f;
    inputs.pitch = utils::approach(inputs.pitch, target_pitch, stick_max * dt);

    float target_yaw = 0.0f;
    if (IsKeyDown(KEY_Q)) target_yaw = 1.0f;
    if (IsKeyDown(KEY_E)) target_yaw = -1.0f;
    inputs.yaw = utils::approach(inputs.yaw, target_yaw, stick_max * dt);

    if (IsKeyPressed(KEY_B)) inputs.brakes = !inputs.brakes;
  }
}
}  // namespace player_systems