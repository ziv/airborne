module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module GearSoundSystem;

import Components;
import Accessors;
import ResourceManager;

// todo put all the gear functionality in one place
export void GearSoundSystem(entt::registry &registry) {
  const auto &inputs = get_player_inputs(registry);
  static bool was_gear_down = true;

  if (inputs.gear != was_gear_down) {
    TraceLog(LOG_WARNING, "gear state changed: %d", inputs.gear);
    was_gear_down = inputs.gear;
    auto &rm = get_resource_manager(registry);
    const auto id = entt::hashed_string("assets/sound/gear-short.wav");
    if (rm.sounds.contains(id)) {
      TraceLog(LOG_WARNING, "playing gear sound");
      // SetSoundVolume(rm.sounds[id]->res, 1.5f);
      PlaySound(rm.sounds[id]->res);
    }
  }
}
