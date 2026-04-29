module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module Prefabs:Scene;

import JsonConfig;
import Components;
import ResourceManager;
import Types;
import Resources;
import Accessors;

export namespace factories {
entt::entity create_scene(entt::registry &registry, const std::string &time_of_day) {
  const auto [mapTexture, mapHeightmap, mapSize, fogShaderVs, fogShaderFs] = get_config(registry).scene;
  auto &assets = get_resource_manager(registry);

  // todo colors should come from config and should be correct (the sunset is not)

  Vector3 dayZenith = {0.1f, 0.3f, 0.9f};    // Dark blue
  Vector3 dayHorizon = {0.6f, 0.8f, 0.99f};  // Light blue

  if (time_of_day == "sunset") {
    dayZenith = {0.1, 0.2f, 0.8f};    // Dark blue
    dayHorizon = {0.8f, 0.5f, 0.8f};  // Purple
  }

  // create sky shaders
  if (!assets.shaders.contains(resources::sky_shader)) {
    const auto sky = LoadShader(resources::sky_vertex_shader_path, resources::sky_fragment_shader_path);
    SetShaderValue(sky, GetShaderLocation(sky, "zenithColor"), &dayZenith, SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, GetShaderLocation(sky, "horizonColor"), &dayHorizon, SHADER_UNIFORM_VEC3);
    assets.shaders.load(resources::sky_shader, sky);
  }

  // create sky model
  if (!assets.models.contains(resources::sky_model)) {
    Mesh sky_mesh = GenMeshSphere(1000.0f, 16, 16);
    Model sky_model = LoadModelFromMesh(sky_mesh);
    sky_model.materials[0].shader = assets.shaders[resources::sky_shader]->res;
    assets.models.load(resources::sky_model, sky_model);
  }

  // play the engine todo move to aircraft systems
  if (assets.music_streams.contains(resources::engine_sound)) {
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
