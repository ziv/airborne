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

Model generate_world_model(const std::string &texture_path, const std::string &heightmap_path, const Vector3 &size, const Shader &fog) {
  // our world texture
  const Image textureImage = LoadImage(texture_path.c_str());
  const Texture2D texture = LoadTextureFromImage(textureImage);
  UnloadImage(textureImage);

  // our world heightmap
  const Image heightImage = LoadImage(heightmap_path.data());
  const Mesh mesh = GenMeshHeightmap(heightImage, size);
  UnloadImage(heightImage);

  // our world model
  const Model model = LoadModelFromMesh(mesh);
  model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
  model.materials[0].shader = fog;

  return model;
}

/**
 * @brief Generate a procedural cloud alpha texture from Perlin noise.
 *
 * Pixels below an intensity threshold become fully transparent (clear sky);
 * brighter pixels become semi-opaque white (clouds). The resulting texture
 * is uploaded to the GPU and the CPU image is freed.
 */
Model init_clouds(const Shader &fog) {
  // noise
  Image noise_image = GenImagePerlinNoise(1024, 1024, 100, 100, 4.0f);

  // allow opacity
  ImageFormat(&noise_image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

  // part of the pixels become clear and part ones become clouds (edit
  //    pixels in place)
  auto *pixels = static_cast<Color *>(noise_image.data);

  for (int i = 0; i < noise_image.width * noise_image.height; i++) {
    if (const unsigned char intensity = pixels[i].r; intensity < 150) {
      // clear sky
      pixels[i] = {255, 255, 255, 0};  // white, but fully transparent
    } else {
      // clouds
      const auto alpha = static_cast<unsigned char>((static_cast<float>(intensity) - 150) * 1.8f);
      pixels[i] = {255, 255, 255, alpha};
    }
  }

  const auto cloud_texture = LoadTextureFromImage(noise_image);
  UnloadImage(noise_image);

  const auto clouds_model = LoadModelFromMesh(GenMeshPlane(200000, 200000, 10, 10));
  clouds_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = cloud_texture;
  clouds_model.materials[0].shader = fog;
  return clouds_model;
}

Shader init_fog(const Vector3 &dayZenith, const Vector3 &dayHorizon) {
  const auto fog = LoadShader("assets/shaders/fog.vs", "assets/shaders/fog.fs");

  constexpr float fogNearValue = 25000.0f;             // start at
  constexpr float fogFarValue = 40000.0f;              // full fogs
  // constexpr Vector3 dayZenith = {0.1f, 0.3f, 0.9f};    // see sky
  // constexpr Vector3 dayHorizon = {0.6f, 0.8f, 0.99f};  // see sky

  SetShaderValue(fog, GetShaderLocation(fog, "zenithColor"), &dayZenith, SHADER_UNIFORM_VEC3);
  SetShaderValue(fog, GetShaderLocation(fog, "horizonColor"), &dayHorizon, SHADER_UNIFORM_VEC3);
  SetShaderValue(fog, GetShaderLocation(fog, "fogNear"), &fogNearValue, SHADER_UNIFORM_FLOAT);
  SetShaderValue(fog, GetShaderLocation(fog, "fogFar"), &fogFarValue, SHADER_UNIFORM_FLOAT);

  return fog;
}

Shader init_sky(const Vector3 &dayZenith, const Vector3 &dayHorizon) {
  const auto sky = LoadShader("assets/shaders/sky.vs", "assets/shaders/sky.fs");

  // constexpr Vector3 dayZenith = {0.1f, 0.3f, 0.9f};    // Dark blue
  // constexpr Vector3 dayHorizon = {0.6f, 0.8f, 0.99f};  // Light blue

  SetShaderValue(sky, GetShaderLocation(sky, "zenithColor"), &dayZenith, SHADER_UNIFORM_VEC3);
  SetShaderValue(sky, GetShaderLocation(sky, "horizonColor"), &dayHorizon, SHADER_UNIFORM_VEC3);

  return sky;
}

export namespace factories {
entt::entity create_scene(entt::registry &registry, const std::string &time_of_day) {
  const auto [mapTexture, mapHeightmap, mapSize, fogShaderVs, fogShaderFs] = get_config(registry).scene;
  auto &assets = get_resource_manager(registry);

  Vector3 dayZenith = {0.1f, 0.3f, 0.9f};    // Dark blue
  Vector3 dayHorizon = {0.6f, 0.8f, 0.99f};  // Light blue

  if (time_of_day == "sunset") {
    dayZenith =  {0.1, 0.2f, 0.8f};  // Dark blue
    dayHorizon = {0.8f, 0.5f, 0.8f};    // Purple
  }

  // create for shader
  if (!assets.shaders.contains(resources::fog_shader)) {
    Shader fog = init_fog(dayZenith, dayHorizon);
    resources::fog_shader_pos_loc = GetShaderLocation(fog, "cameraPos");
    assets.shaders.load(resources::fog_shader, fog);
  }

  // create sky shader
  if (!assets.shaders.contains(resources::sky_shader)) {
    assets.shaders.load(resources::sky_shader, init_sky(dayZenith, dayHorizon));
  }

  // create sky model
  if (!assets.models.contains(resources::sky_model)) {
    Mesh sky_mesh = GenMeshSphere(1000.0f, 16, 16);
    Model sky_model = LoadModelFromMesh(sky_mesh);
    sky_model.materials[0].shader = assets.shaders[resources::sky_shader]->res;
    assets.models.load(resources::sky_model, sky_model);
  }

  //
  // if (!assets.models.contains(resources::world_model)) {
  //   assets.models.load(resources::world_model, generate_world_model(mapTexture, mapHeightmap, mapSize, fog));
  //   TraceLog(LOG_DEBUG, "world model created");
  // }
  //
  //
  // if (!assets.models.contains(resources::cloud_model)) {
  //   assets.models.load(resources::cloud_model, init_clouds(fog));
  //   TraceLog(LOG_DEBUG, "cloud model created");
  // }

  if (assets.music_streams.contains(resources::engine_sound)) {
    PlayMusicStream(assets.music_streams[resources::engine_sound]->res);
    TraceLog(LOG_DEBUG, "engine sound played");
  } else {
    TraceLog(LOG_WARNING, "engine sound resource not found");
  }

  const auto entity = registry.create();

  // registry.emplace<World>(entity, assets.models[resources::world_model], assets.models[resources::cloud_model],
  // assets.music_streams[resources::engine_sound]); registry.emplace<Position3D>(entity, Vector3Zero(), Vector3Zero());

  // registry.emplace<EngineState>(entity, )
  TraceLog(LOG_DEBUG, "scene created");
  return entity;
}
}  // namespace factories
