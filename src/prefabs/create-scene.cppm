module;
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include "../lib/ray.hpp"

export module Prefabs:Scene;

import ResourceManager;
import Resources;

export namespace factories {
entt::entity create_scene(entt::registry &registry, const nlohmann::json &scenario) {
  auto &assets = get_resource_manager(registry);

  // create sky shaders
  if (!assets.shaders.contains(resources::sky_shader)) {
    const Vector3 dayZenith = scenario["sky"]["zenith_color"].get<Vector3>();
    const Vector3 dayHorizon = scenario["sky"]["horizon_color"].get<Vector3>();

    const auto sky = LoadShader(resources::sky_vertex_shader_path, resources::sky_fragment_shader_path);
    SetShaderValue(sky, GetShaderLocation(sky, "zenithColor"), &dayZenith, SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, GetShaderLocation(sky, "horizonColor"), &dayHorizon, SHADER_UNIFORM_VEC3);
    assets.shaders.load(resources::sky_shader, sky);
  }

  // create sky model
  if (!assets.models.contains(resources::sky_model)) {
    const Mesh sky_mesh = GenMeshSphere(1000.0f, 16, 16);
    Model sky_model = LoadModelFromMesh(sky_mesh);
    sky_model.materials[0].shader = assets.shaders[resources::sky_shader]->res;
    assets.models.load(resources::sky_model, sky_model);
  }

  // play the engine todo move to aircraft systems
  if (assets.music_streams.contains(resources::engine_sound)) {
    TraceLog(LOG_ERROR, "playing engine sound");
    PlayMusicStream(assets.music_streams[resources::engine_sound]->res);
    TraceLog(LOG_DEBUG, "engine sound played");
  } else {
    TraceLog(LOG_WARNING, "engine sound resource not found");
  }

  // todo do we need this?
  const auto entity = registry.create();
  TraceLog(LOG_DEBUG, "scene created");
  return entity;
}
}  // namespace factories
