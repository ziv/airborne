module;
#include <memory>
#include <string>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include "../lib/ray.hpp"

// todo should be template

export module ResourcePreloader;

import JsonConfig;
import ResourceManager;

export struct ResourceDef {
    std::string name;
    std::string type;
    std::string path;
};

export
{
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ResourceDef, name, type, path)
}

export void preload_resources(entt::registry &registry) {
    const JsonConfig resources("assets/resources.jsonc");
    auto &rm = get_resource_manager(registry);

    for (const auto items = resources.get<std::vector<ResourceDef> >("/resources");
         const auto &[name, type, path]: items) {
        TraceLog(LOG_DEBUG, TextFormat("preloading resource '%s' of type '%s' from path '%s'", name.c_str(), type.c_str(), path.c_str()));

        const auto res_id = entt::hashed_string(path.data());
        TraceLog(LOG_DEBUG, "LOADING: String='%s' | Hash=%u",
                 path.c_str(), res_id.value());
        if (type == "texture") {
            rm.textures.load(res_id, path);
        } else if (type == "model") {
            rm.models.load(res_id, path);
        } else if (type == "image") {
            rm.images.load(res_id, path);
        } else if (type == "fragment") {
            rm.shaders.load(res_id, path);
        } else if (type == "music") {
            rm.music_streams.load(res_id, path);
        } else {
            TraceLog(LOG_WARNING, TextFormat("Unknown resource type '%s' for resource '%s'", type.c_str(), name.c_str()));
        }
    }
    TraceLog(LOG_INFO, "all resources loaded");
}
