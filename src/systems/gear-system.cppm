module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module GearSoundSystem;

import Accessors;
import ResourceManager;
import Resources;

/// @brief Toggles the landing gear and plays a sound when the player presses the G key while flying.
export void GearSystem(entt::registry &registry) {
  if (!IsKeyPressed(KEY_G) || !is_player_flying(registry)) return;

  auto &inputs = get_player_inputs(registry);
  inputs.gear = !inputs.gear;

  auto &rm = get_resource_manager(registry);
  if (!rm.sounds.contains(resources::gear_sound)) return;

  PlaySound(rm.sounds[resources::gear_sound]->res);
}
