module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"
#include "rlgl.h"

export module RenderSystem:Models;

import Components;
import ResourceManager;
import Accessors;
import Resources;

export void RenderModels(entt::registry &registry, const Camera3D &camera) {
  const auto view = registry.view<Position3D, WithModel, Heading>(entt::exclude<World>);
  const auto &models = get_resource_manager(registry).models;

  for (auto [entity, position, modeled, heading] : view.each()) {

    // 1. model not exists
    if (!models.contains(modeled.model)) continue;

    const auto offset = get_player(registry).offset;
    const auto model_position = position.pos + offset;
    // const auto &player_position = get_player(registry).pos + offset;
    //
    // // 2. model too far
    // // todo check the distance if it valid....(render real model in 200000)
    // if (Vector3Distance(model_position, player_position) > 20000.0f) continue;
    //
    // // 3. is the model in view
    // const Vector3 direction_to_entity = Vector3Normalize(Vector3Subtract(model_position, camera.position));
    // const Vector3 camera_forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));  // todo player forward?!
    //
    // if (Vector3DotProduct(camera_forward, direction_to_entity) < 0.0f) continue;

    DrawModelEx(models[modeled.model]->res, model_position, {0.0f, 1.0f, 0.0f}, heading.heading, {1.0f, 1.0f, 1.0f}, WHITE);
  }
}

export namespace render_systems {

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