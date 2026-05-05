module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"
#include "rlgl.h"

export module RenderSystem:Models;

import Components;
import ResourceManager;
import Accessors;
import Resources;

constexpr float MODEL_RENDER_DISTANCE_SQR = 500000000.0f;  // 22360^2
constexpr float MODEL_ALLWAYS_DISTANCE_SQR = 250000.0f;    // 500^2

export namespace render_systems {

/// @brief render all models in the world, with distance and angle culling
void models(entt::registry &registry) {
  const auto &models = get_resource_manager(registry).models;
  const auto &player = get_player(registry);
  const auto &player_position = player.pos;

  // DrawLine3D(player.pos, Vector3Add(player.pos, Vector3Scale(player.forward, 5000.0f)), RED);
  // TraceLog(LOG_DEBUG, "rendering models, player at (%.1f, %.1f, %.1f)", player.forward.x, player.forward.y, player.forward.z);


  for (const auto view = registry.view<Position3D, WithModel, Heading>(); auto [entity, position, modeled, heading] : view.each()) {
    // 1. model not exists
    if (!models.contains(modeled.model)) continue;

    // 2. model too far
    const auto model_position = position.pos + player.offset;  // model position to user coordinates
    const auto distance = Vector3DistanceSqr(model_position, player_position);
    if (distance > MODEL_RENDER_DISTANCE_SQR) continue;

    // 3. model not in front of us but for close items, they still rendered
    if (const Vector3 direction_to_entity = Vector3Normalize(Vector3Subtract(model_position, player_position));
        distance > MODEL_ALLWAYS_DISTANCE_SQR && Vector3DotProduct(player.forward, direction_to_entity) < 0.0f)
      continue;

    DrawModelEx(models[modeled.model]->res, model_position, {0.0f, 1.0f, 0.0f}, heading.heading, {1.0f, 1.0f, 1.0f}, WHITE);
  }
}

/// @brief render the sky model as background
/// sky is a sphere that with disable depth become the background
/// of the 3d world. the shader is responsible to the colors and
/// clouds animation
void sky(entt::registry &registry) {
  const auto &player = get_player(registry);
  const auto &assets = get_resource_manager(registry);
  auto &options = get_options(registry);

  const auto t = static_cast<float>(GetTime());
  const Shader &sky_shader = assets.shaders[resources::sky_shader]->res;

  static int time_loc = GetShaderLocation(sky_shader, "time");
  static int zenith_loc = GetShaderLocation(sky_shader, "zenithColor");
  static int horizon_loc = GetShaderLocation(sky_shader, "horizonColor");

  SetShaderValue(sky_shader, time_loc, &t, SHADER_UNIFORM_FLOAT);
  SetShaderValue(sky_shader, zenith_loc, &options.get_zenith_color(), SHADER_UNIFORM_VEC3);
  SetShaderValue(sky_shader, horizon_loc, &options.get_horizon_color(), SHADER_UNIFORM_VEC3);

  rlDisableDepthTest();
  rlDisableBackfaceCulling();
  DrawModel(assets.models[resources::sky_model]->res, player.pos, 1.0f, WHITE);
  rlEnableBackfaceCulling();
  rlEnableDepthTest();
}
}  // namespace render_systems