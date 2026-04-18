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

export namespace factories {
    void create_cockpit(entt::registry &registry,
                       const JsonConfig &config) {
        const auto conf = config.get<CockpitConfig>("/views/cockpit");
        auto &manager = get_resource_manager(registry);

        const auto cockpit = registry.create();
        registry.emplace<CockpitWidget>(cockpit);
        registry.emplace<Position2D>(cockpit, (Vector2){0.0f, 0.0});

        if (const auto tex_id = entt::hashed_string(conf.texturePath.c_str()); manager.textures.contains(tex_id)) {
            registry.emplace<WithTexture>(cockpit, manager.textures[tex_id]);
        } else {
            TraceLog(LOG_WARNING, "cockpit texture '%s' not found in cache", conf.texturePath.c_str());
        }

        if (const auto fs_id = entt::hashed_string(conf.shaderPath.c_str()); manager.shaders.contains(fs_id)) {
            registry.emplace<WithFsShader>(cockpit, manager.shaders[fs_id]);
        } else {
            TraceLog(LOG_WARNING, "cockpit shader '%s' not found in cache", conf.shaderPath.c_str());
        }
    }
}
