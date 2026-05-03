module;
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include "../lib/ray.hpp"

export module Prefabs:Scene;

import ResourceManager;
import Resources;

export namespace factories {

void create_engine(entt::registry &registry) {
  // play the engine
  if (auto &assets = get_resource_manager(registry); assets.music_streams.contains(resources::engine_sound)) {
    TraceLog(LOG_ERROR, "playing engine sound");
    PlayMusicStream(assets.music_streams[resources::engine_sound]->res);
    TraceLog(LOG_DEBUG, "engine sound played");
  } else {
    TraceLog(LOG_WARNING, "engine sound resource not found");
  }
}

void create_scene(entt::registry &registry, const nlohmann::json &scene) {
  auto &assets = get_resource_manager(registry);

  // create sky shaders
  if (!assets.shaders.contains(resources::sky_shader)) {
    const Vector3 day_zenith = scene["sky"]["zenith_color"].get<Vector3>();
    const Vector3 day_horizon = scene["sky"]["horizon_color"].get<Vector3>();

    const auto sky = LoadShader(resources::sky_vertex_shader_path, resources::sky_fragment_shader_path);
    SetShaderValue(sky, GetShaderLocation(sky, "zenithColor"), &day_zenith, SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, GetShaderLocation(sky, "horizonColor"), &day_horizon, SHADER_UNIFORM_VEC3);

    // todo add variable to control the clouds color
    // todo add color ambient like in terrain
    // SetShaderValue(sky, GetShaderLocation(sky, "cloudColor"), &day_horizon, SHADER_UNIFORM_VEC3);
    assets.shaders.load(resources::sky_shader, sky);
  }

  // create sky model
  if (!assets.models.contains(resources::sky_model)) {
    const Mesh sky_mesh = GenMeshSphere(1000.0f, 16, 16);
    Model sky_model = LoadModelFromMesh(sky_mesh);
    sky_model.materials[0].shader = assets.shaders[resources::sky_shader]->res;
    assets.models.load(resources::sky_model, sky_model);
  }
}
}  // namespace factories
