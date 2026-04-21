module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module FlightControlsSystem;

import Accessors;
import ResourceManager;
import Resources;

export void FlightControlsSystem(entt::registry &registry) {
  auto &inputs = get_player_inputs(registry);

  
}
