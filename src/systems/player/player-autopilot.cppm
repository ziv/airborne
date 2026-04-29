module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module PlayerSystems:Autopilot;

import Components;
import Types;
import Utils;

export namespace player_systems {

void autopilot(entt::registry& registry, const float dt) {
  // this will steer the aircraft as long as there is no autopilot set
  for (const auto view = registry.view<Player, PlayerInputs, Autopilot>(); auto [entity, player, inputs] : view.each()) {
    //   constexpr auto stick_max = 3.0f;
    //   float targetRoll = 0.0f;
    //   if (IsKeyDown(KEY_RIGHT)) targetRoll = 1.0f;
    //   if (IsKeyDown(KEY_LEFT)) targetRoll = -1.0f;
    //   inputs.roll = utils::approach(inputs.roll, targetRoll, stick_max * dt);
    //
    //   float targetPitch = 0.0f;
    //   if (IsKeyDown(KEY_UP)) targetPitch = -1.0f;
    //   if (IsKeyDown(KEY_DOWN)) targetPitch = 1.0f;
    //   inputs.pitch = utils::approach(inputs.pitch, targetPitch, stick_max * dt);
    //
    //   float targetYaw = 0.0f;
    //   if (IsKeyDown(KEY_Q)) targetYaw = 1.0f;
    //   if (IsKeyDown(KEY_E)) targetYaw = -1.0f;
    //   inputs.yaw = utils::approach(inputs.yaw, targetYaw, stick_max * dt);
    //
    //   if (IsKeyPressed(KEY_B)) inputs.brakes = !inputs.brakes;
    // }
  }
}  // namespace player_systems