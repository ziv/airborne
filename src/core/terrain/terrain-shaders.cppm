module;
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include "../../lib/ray.hpp"
export module TerrainShaders;

import Accessors;
import Resources;
import ResourceManager;

export namespace terrain_streamer {

/// sky shader
void load_sky_shader(entt::registry& registry, const nlohmann::json& sky_config) {
  const auto sky = LoadShader(sky_config["vertex"].get<std::string>().c_str(), sky_config["fragment"].get<std::string>().c_str());

  const auto zenith_color = sky_config["zenith_color"].get<Vector3>();
  const auto horizon_color = sky_config["horizon_color"].get<Vector3>();

  SetShaderValue(sky, GetShaderLocation(sky, "zenithColor"), &zenith_color, SHADER_UNIFORM_VEC3);
  SetShaderValue(sky, GetShaderLocation(sky, "horizonColor"), &horizon_color, SHADER_UNIFORM_VEC3);

  get_resource_manager(registry).shaders.load(resources::sky_shader, sky);
}
}  // namespace terrain_streamer

Shader load_fog_shader(const Vector3& dayZenith, const Vector3& dayHorizon) {
  const auto fog = LoadShader("assets/shaders/fog.vs", "assets/shaders/fog.fs");

  constexpr float fogNearValue = 20000.0f;  // start at
  constexpr float fogFarValue = 40000.0f;   // full fogs
  // constexpr Vector3 dayZenith = {0.1f, 0.3f, 0.9f};    // see sky
  // constexpr Vector3 dayHorizon = {0.6f, 0.8f, 0.99f};  // see sky

  SetShaderValue(fog, GetShaderLocation(fog, "zenithColor"), &dayZenith, SHADER_UNIFORM_VEC3);
  SetShaderValue(fog, GetShaderLocation(fog, "horizonColor"), &dayHorizon, SHADER_UNIFORM_VEC3);
  SetShaderValue(fog, GetShaderLocation(fog, "fogNear"), &fogNearValue, SHADER_UNIFORM_FLOAT);
  SetShaderValue(fog, GetShaderLocation(fog, "fogFar"), &fogFarValue, SHADER_UNIFORM_FLOAT);

  return fog;
}  // namespace terrain_streamer