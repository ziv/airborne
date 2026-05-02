module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"
#include "rlgl.h"

export module RenderSystem:Models;

import Components;
import ResourceManager;
import Accessors;
import Resources;

constexpr float MODEL_RENDER_DISTANCE_SQR = 400000000.0f;  // 20000^2

export namespace render_systems {

void models(entt::registry &registry) {
  const auto view = registry.view<Position3D, WithModel, Heading>(entt::exclude<World>);
  const auto &models = get_resource_manager(registry).models;
  const auto &player = get_player(registry);
  const auto &player_position = player.abs_pos;

  for (auto [entity, position, modeled, heading] : view.each()) {
    // 1. model not exists
    if (!models.contains(modeled.model)) continue;

    // 2. model too far
    const auto model_position = position.pos + player.offset;
    // if (Vector3DistanceSqr(model_position, player_position) > MODEL_RENDER_DISTANCE_SQR) continue;

    // 3. model not in front of us
    // const Vector3 direction_to_entity = Vector3Normalize(Vector3Subtract(model_position, player_position));
    // if (Vector3DotProduct(player.forward, direction_to_entity) < 0.0f) continue;

    DrawModelEx(models[modeled.model]->res, model_position, {0.0f, 1.0f, 0.0f}, heading.heading, {1.0f, 1.0f, 1.0f}, WHITE);
  }
}

/// @brief render the sky model as background
void sky(entt::registry &registry) {
  const auto &player = get_player(registry);
  const auto &assets = get_resource_manager(registry);
  const auto &models = assets.models;

  const auto t = static_cast<float>(GetTime());
  const Shader &sky_shader = assets.shaders[resources::sky_shader]->res;
  static int time_loc = GetShaderLocation(sky_shader, "time");
  SetShaderValue(sky_shader, time_loc, &t, SHADER_UNIFORM_FLOAT);

  rlDisableDepthTest();
  rlDisableBackfaceCulling();
  DrawModel(models[resources::sky_model]->res, player.pos, 1.0f, WHITE);
  rlEnableBackfaceCulling();
  rlEnableDepthTest();
}
}  // namespace render_systems