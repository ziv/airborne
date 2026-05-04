module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module RenderSystem:Minimap;

import Components;
import ResourceManager;
import MapStreaming;
import Accessors;

constexpr float TILE_SIZE_Z12 = 8300.0f;
constexpr float MAP_BASE_X = 2444.0f;
constexpr float MAP_BASE_Z = 1655.0f;

// All rendering is in "geo-pixel" space: geo tile g drawn at (g*256, g*256).
// Camera target is the player's continuous position in that same space.
// This ensures tile content and player position are always aligned, even at
// zoom < 12 where local-index space and geo space diverge (BASE not divisible
// by 2^(12-zoom)).
constexpr float TILE_PX = 256.0f;
constexpr float ORIGIN_X = MAP_BASE_X * TILE_SIZE_Z12;  // world x where geo tile 0 starts (BASE_X * TILE_Z12)
constexpr float ORIGIN_Z = MAP_BASE_Z * TILE_SIZE_Z12;  // world z where geo tile 0 starts (BASE_Z * TILE_Z12)

static float minimap_tile_size(const int zoom) { return TILE_SIZE_Z12 * static_cast<float>(std::pow(2.0f, 12 - zoom)); }

void draw_aircraft_marker(const Player& player, const Vector2& target) {
  const float heading = 180.0f - atan2f(player.forward.x, player.forward.z) * RAD2DEG;
  constexpr float tri_w = 5.0f;
  constexpr float tri_h = 12.0f;
  Vector2 v1 = {0.0f, -tri_h * 0.5f};
  Vector2 v2 = {-tri_w * 0.5f, tri_h * 0.5f};
  Vector2 v3 = {tri_w * 0.5f, tri_h * 0.5f};
  v1 = Vector2Add(Vector2Rotate(v1, heading * DEG2RAD), target);
  v2 = Vector2Add(Vector2Rotate(v2, heading * DEG2RAD), target);
  v3 = Vector2Add(Vector2Rotate(v3, heading * DEG2RAD), target);
  DrawTriangle(v1, v2, v3, GREEN);
  DrawTriangleLines(v1, v2, v3, BLACK);
}

export void RenderMinimap(entt::registry& registry) {
  const auto& player = get_player(registry);
  for (const auto view = registry.view<const DashboardSlot, const MinimapWidget, const Position2D>(); auto [entity, slot, wd, pos] : view.each()) {
    const int map_zoom = wd.map_zoom;
    const float tile_sz = minimap_tile_size(map_zoom);

    const int widget_x = static_cast<int>(pos.pos.x);
    const int widget_y = static_cast<int>(pos.pos.y);
    const int size = wd.cfg.size;
    const float half = static_cast<float>(size) * 0.5f;

    Camera2D cam{};
    cam.target = {(player.abs_pos.x + ORIGIN_X) / tile_sz * TILE_PX, (player.abs_pos.z + ORIGIN_Z) / tile_sz * TILE_PX};
    cam.zoom = 1.0f;
    cam.rotation = 0.0f;                                // north-up
    cam.offset = {pos.pos.x + half, pos.pos.y + half};  // center of the screen

    BeginScissorMode(widget_x, widget_y, size, size);
    DrawRectangle(widget_x, widget_y, size, size, DARKGRAY);
    BeginMode2D(cam);

    // Draw each loaded tile at its geographic position.
    const auto& rm = get_resource_manager(registry);
    for (const auto tile_view = registry.view<MapTile>(); const auto [te, tile] : tile_view.each()) {
      if (tile.zoom != map_zoom) continue;
      if (!rm.textures.contains(tile.tex_id)) continue;

      const Texture2D& tex = rm.textures[tile.tex_id]->res;
      DrawTexture(tex, tile.geo_x * static_cast<int>(TILE_PX), tile.geo_z * static_cast<int>(TILE_PX), WHITE);
    }

    draw_aircraft_marker(player, cam.target);
    EndMode2D();
    DrawText(TextFormat("Z[%d]", map_zoom), widget_x + 5, widget_y + size - 14, 10, WHITE);
    EndScissorMode();
  }
}