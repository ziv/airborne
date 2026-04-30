module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module RenderSystem:TargetCamera;

import Components;
import Helpers;
import Accessors;
import Types;
import ResourceManager;

export namespace render_systems {
void target_camera(entt::registry &registry) {
  const auto view = registry.view<DashboardSlot, TargetCameraWidget, Position2D>();
  const auto &player = get_player(registry);
  const auto &radar = registry.get<RadarState>(get_player_entity(registry));
  const auto &rm = get_resource_manager(registry);

  for (auto [entity, slot, wd, pos] : view.each()) {
    const int wx = static_cast<int>(pos.pos.x);
    const int wy = static_cast<int>(pos.pos.y);
    const int sz = wd.size;

    if (!rm.render_textures.contains(wd.render_tex_id)) continue;
    const RenderTexture2D &rt = rm.render_textures[wd.render_tex_id]->res;

    if (radar.locked_target == entt::null || !registry.valid(radar.locked_target)) {
      DrawRectangle(wx, wy, sz, sz, BLACK);
      DrawText("NO TARGET", wx + 10, wy + sz / 2 - 8, 12, DARKGREEN);
      continue;
    }

    const auto &target_pos = registry.get<Position3D>(radar.locked_target).pos;
    const auto &target_type = registry.get<IdentifyType>(radar.locked_target).type;
    const Vector3 player_abs = player.absolute_position();
    const float distance = Vector3Distance(player_abs, target_pos);
    const int alt_feet = static_cast<int>(meter_to_feet(target_pos.y));

    float distance_to_put_camera, fovy;
    switch (target_type) {
      case EntityType::CARRIER:
      case EntityType::STRUCTURE:
      case EntityType::AIRBASE:
        distance_to_put_camera = 350.0f;
        fovy = 35.0;
        break;
      default:
        distance_to_put_camera = 100.0f;
        fovy = 20.0;
    }

    // place camera "distance_to_put_camera" m from target along the player→target line
    const Vector3 to_target = Vector3Normalize(target_pos - player_abs);
    const Vector3 cam_pos = target_pos - Vector3Scale(to_target, distance_to_put_camera);

    Camera3D cam{};
    cam.position = cam_pos + player.offset;
    cam.target = target_pos + player.offset;
    cam.up = world_up();
    cam.fovy = fovy;
    cam.projection = CAMERA_PERSPECTIVE;

    BeginTextureMode(rt);
    ClearBackground({10, 10, 20, 255});
    BeginMode3D(cam);
    if (const auto *modeled = registry.try_get<WithModel>(radar.locked_target)) {
      if (rm.models.contains(modeled->model)) {
        const float hdg = registry.all_of<Heading>(radar.locked_target) ? registry.get<Heading>(radar.locked_target).heading : 0.0f;
        DrawModelEx(rm.models[modeled->model]->res, target_pos + player.offset, {0.0f, 1.0f, 0.0f}, hdg, {1.0f, 1.0f, 1.0f}, WHITE);
      }
    }
    EndMode3D();
    EndTextureMode();

    const Rectangle src = {0, 0, static_cast<float>(sz), -static_cast<float>(sz)};
    const Rectangle dest = {static_cast<float>(wx), static_cast<float>(wy), static_cast<float>(sz), static_cast<float>(sz)};
    DrawTexturePro(rt.texture, src, dest, {0, 0}, 0.0f, WHITE);

    DrawRectangleLines(wx, wy, sz, sz, DARKGREEN);
    if (registry.all_of<Identify>(radar.locked_target)) DrawText(registry.get<Identify>(radar.locked_target).name.c_str(), wx + 4, wy + 4, 10, GREEN);
    DrawText(TextFormat("%.1f km", distance / 1000.0f), wx + 4, wy + sz - 28, 10, GREEN);
    DrawText(TextFormat("%d ft", alt_feet), wx + 4, wy + sz - 16, 10, GREEN);
  }
}
}  // namespace render_systems