module;
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include "../lib/ray.hpp"

export module Prefabs:Cockpit;

import JsonConfig;
import Components;
import RaylibResource;
import ResourceManager;
import Types;
import Accessors;

export namespace factories {
void create_cockpit(entt::registry &registry) {
  const auto conf = get_config(registry).views.cockpit;
  const auto &manager = get_resource_manager(registry);

  const auto cockpit = registry.create();
  registry.emplace<CockpitWidget>(cockpit);
  registry.emplace<Position2D>(cockpit, (Vector2){0.0f, 0.0});

  if (const auto tex_id = entt::hashed_string(conf.texture_path.c_str()); manager.textures.contains(tex_id)) {
    registry.emplace<WithTexture>(cockpit, tex_id);
  } else {
    TraceLog(LOG_WARNING, "cockpit texture '%s' not found in cache", conf.texture_path.c_str());
  }

  if (const auto fs_id = entt::hashed_string(conf.shader_path.c_str()); manager.shaders.contains(fs_id)) {
    registry.emplace<WithFsShader>(cockpit, fs_id);
  } else {
    TraceLog(LOG_WARNING, "cockpit shader '%s' not found in cache", conf.shader_path.c_str());
  }
}
}  // namespace factories
