module;
#include <entt/entt.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "../../lib/ray.hpp"

export module ResourcePreloader;

import ResourceManager;
import Types;

export void preload_resource(ResourceManager &rm, const ResourceDef &def) {
  const auto [name, type, path] = def;
  TraceLog(LOG_DEBUG, TextFormat("preloading resource '%s' of type '%s' from path '%s'", name.c_str(), type.c_str(), path.c_str()));

  const auto res_id = entt::hashed_string(path.data());
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
  } else if (type == "sound") {
    rm.sounds.load(res_id, path);
  } else {
    TraceLog(LOG_WARNING, TextFormat("Unknown resource type '%s' for resource '%s'", type.c_str(), name.c_str()));
  }
}
