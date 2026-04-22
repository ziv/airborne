module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module AircraftSystems:Engine;

import Components;
import Accessors;
import ResourceManager;
import Resources;

export namespace aircraft_systems {
void engine(entt::registry &registry, const float dt) {
  auto &inputs = get_player_inputs(registry);

  // inputs
  if (IsKeyDown(KEY_MINUS)) inputs.throttle -= 0.2f * dt;
  if (IsKeyDown(KEY_EQUAL)) inputs.throttle += 0.2f * dt;

  if (IsKeyDown(KEY_A)) inputs.throttle = 1.2f;
  if (IsKeyPressed(KEY_ZERO)) inputs.throttle = 0.0f;
  if (IsKeyPressed(KEY_ONE)) inputs.throttle = 0.1f;
  if (IsKeyPressed(KEY_TWO)) inputs.throttle = 0.2f;
  if (IsKeyPressed(KEY_THREE)) inputs.throttle = 0.3f;
  if (IsKeyPressed(KEY_FOUR)) inputs.throttle = 0.4f;
  if (IsKeyPressed(KEY_FIVE)) inputs.throttle = 0.5f;
  if (IsKeyPressed(KEY_SIX)) inputs.throttle = 0.6f;
  if (IsKeyPressed(KEY_SEVEN)) inputs.throttle = 0.7f;
  if (IsKeyPressed(KEY_EIGHT)) inputs.throttle = 0.8f;
  if (IsKeyPressed(KEY_NINE)) inputs.throttle = 0.9f;

  inputs.throttle = std::clamp(inputs.throttle, 0.0f, 1.2f);

  // sound
  if (0.0f == inputs.throttle) return;

  const auto &rm = get_resource_manager(registry);
  if (!rm.music_streams.contains(resources::engine_sound)) return;

  const auto &res = rm.music_streams[resources::engine_sound]->res;

  const float target_pitch = 0.8f + (inputs.throttle * 0.7f);
  const float target_volume = 0.2f + (inputs.throttle * 0.9f);

  SetMusicPitch(res, target_pitch);
  SetMusicVolume(res, target_volume);
  UpdateMusicStream(res);
}
}  // namespace aircraft_systems
