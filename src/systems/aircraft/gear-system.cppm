module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module AircraftSystems:Gear;

import Accessors;
import ResourceManager;
import Resources;

export namespace aircraft_systems {
/// @brief Toggles the landing gear and plays a sound when the player presses the G key while flying.
void gear(entt::registry &registry) {
  if (IsKeyPressed(KEY_G) && !is_player_flying(registry)) {
    auto &inputs = get_player_inputs(registry);
    inputs.gear = !inputs.gear;

    auto &rm = get_resource_manager(registry);
    if (rm.sounds.contains(resources::gear_sound)) PlaySound(rm.sounds[resources::gear_sound]->res);
  }
}
}  // namespace aircraft_systems