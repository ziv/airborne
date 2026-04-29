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

  Pixel centerX = global.width / 2;
  Pixel centerY = global.height / 2;
  Pixel pixelsPerDegree = static_cast<Pixel>(static_cast<float>(global.height) / global.fov);
  float ppd = static_cast<float>(global.height) / global.fov;

  const auto hud = registry.create();
  registry.emplace<HudWidget>(hud, cfg, centerX, centerY, pixelsPerDegree, ppd, global.tilt, 0);
}
}  // namespace factories
