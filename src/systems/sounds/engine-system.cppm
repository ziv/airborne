module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module EngineSoundSystem;

import Components;
import Accessors;

export void EngineSoundSystem(entt::registry &registry) {
  const auto &inputs = get_player_inputs(registry);

  for (const auto view = registry.view<const World>(); auto [entity, world] : view.each()) {
    // the "background"
    if (inputs.throttle > 0 && world.streams) {
      const float target_pitch = 0.8f + (inputs.throttle * 0.7f);
      const float target_volume = 0.2f + (inputs.throttle * 0.9f);
      SetMusicPitch(world.streams->res, target_pitch);
      SetMusicVolume(world.streams->res, target_volume);
      UpdateMusicStream(world.streams->res);
    }
  }
}
