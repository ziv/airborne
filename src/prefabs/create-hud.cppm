module;
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include "../lib/ray.hpp"

export module Prefabs:Hud;

import JsonConfig;
import Components;
import RaylibResource;
import ResourceManager;
import Types;
import Accessors;

export namespace factories {
void create_hud(entt::registry& registry) {
  const auto global = get_config(registry).global;
  const auto cfg = get_config(registry).views.hud;

  Pixel center_x = global.width / 2;
  Pixel center_y = global.height / 2;
  Pixel pixels_per_degree = static_cast<Pixel>(static_cast<float>(global.height) / global.fov);
  float ppd = static_cast<float>(global.height) / global.fov;

  const auto hud = registry.create();
  registry.emplace<HudWidget>(hud, cfg, center_x, center_y, pixels_per_degree, ppd, global.tilt, 0);
}
}  // namespace factories
