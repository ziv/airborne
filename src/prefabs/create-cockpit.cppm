module;
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include "../lib/ray.hpp"

export module Prefabs:Cockpit;

import JsonConfig;
import Components;
import RaylibResource;
import ResourceManager;
import Resources;
import Types;
import Accessors;

export namespace factories {
void create_cockpit(entt::registry &registry) {
  // currently only attach resources
  registry.ctx().emplace<CockpitWidget>(resources::cockpit_texture, resources::cockpit_chroma);
}
}  // namespace factories
