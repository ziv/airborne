module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module RenderSystem:Radar;

import Components;
import Helpers;
import Accessors;
import Types;

void draw_scope(const Vector2 &center, const float display_radius, const float range) {
  const auto x = static_cast<int>(center.x);
  const auto y = static_cast<int>(center.y);
  const auto r = static_cast<int>(display_radius);

  DrawCircleLines(x, y, display_radius / 3.0f, DARKGREEN);
  DrawCircleLines(x, y, display_radius * 2.0f / 3.0f, DARKGREEN);
  DrawCircleLines(x, y, display_radius, DARKGREEN);

  // nose line
  DrawLine(x, y, x, y - r, {0, 100, 0, 255});

  const float range_nm = meter_to_nm(range);
  DrawText(TextFormat("%.0f NM", range_nm), x - r + 10, y + r - 10, 10, DARKGREEN);
}

/// @brief Aircraft blip is a triangle
void draw_aircraft(int x, int y, float heading, const Color &color) {
  const auto fx = static_cast<float>(x);
  const auto fy = static_cast<float>(y);
  const auto rad = heading * DEG2RAD;
  const Vector2 fwd = {sinf(rad), -cosf(rad)};
  const Vector2 right = {fwd.y, -fwd.x};

  const Vector2 p1 = {fx + fwd.x * 6, fy + fwd.y * 6};
  const Vector2 p2 = {fx - fwd.x * 4 + right.x * 3, fy - fwd.y * 4 + right.y * 3};
  const Vector2 p3 = {fx - fwd.x * 4 - right.x * 3, fy - fwd.y * 4 - right.y * 3};

  DrawTriangle(p1, p2, p3, color);
}

/// @brief Ship blip is a sircle
void draw_ship(int x, int y, const Color &color) { DrawCircle(x, y, 5, color); }

/// @brief SAM/AAA blip is an X
void draw_sam(int x, int y, const Color &color) {
  const auto fx = static_cast<float>(x);
  const auto fy = static_cast<float>(y);
  DrawLineEx({fx - 3, fy - 3}, {fx + 3, fy + 3}, 2, color);
  DrawLineEx({fx - 3, fy + 3}, {fx + 3, fy - 3}, 2, color);
}

/// @brief SAM/AAA blip is a rectangle
void draw_structure(int x, int y, const Color &color) { DrawRectangle(x - 2, y - 2, 4, 4, color); }

export void render_radar(entt::registry &registry) {
  const auto view = registry.view<DashboardSlot, RadarWidget, Position2D>();
  const auto &player = get_player(registry);

  for (auto [entity, slot, wd, pos] : view.each()) {
    const auto display_radius = static_cast<float>(wd.cfg.size) / 2.0f;
    const Vector2 center = {pos.pos.x + display_radius, pos.pos.y + display_radius};
    const float range = wd.cfg.ranges[wd.range_index];

    draw_scope(center, display_radius, range);

    const float range_sq = range * range;
    const float pixels_per_meter = display_radius / range;
    auto color = GRAY;

    // player absolute world position (accounting for large-world offset)
    const float playerX = player.pos.x - player.offset.x;
    const float playerZ = player.pos.z - player.offset.z;
    const float player_h = atan2f(player.forward.x, player.forward.z) * RAD2DEG;

    // project orientation onto XZ plane for top-down radar
    const Vector2 fwd_xz = {player.forward.x, player.forward.z};
    const Vector2 fwd = Vector2LengthSqr(fwd_xz) > 0.0001f ? Vector2Normalize(fwd_xz) : Vector2{0.0f, 1.0f};
    const Vector2 right = {-fwd.y, fwd.x};

    const auto &radar = registry.get<RadarState>(get_player_entity(registry));

    // iterating items and if they are in range, display them on the radar
    for (const auto blip_view = registry.view<Identify, Position3D, IdentifyType, FriendFoe, Heading>();
         const auto [en, id, pos, typ, ff, hd] : blip_view.each()) {
      const float dx = pos.pos.x - playerX;
      const float dz = pos.pos.z - playerZ;

      const float along_fwd = dx * fwd.x + dz * fwd.y;
      const float along_right = dx * right.x + dz * right.y;

      if (along_fwd * along_fwd + along_right * along_right > range_sq) continue;

      // const Vector2 blipPos = {center.x + along_right * pixels_per_meter, center.y - along_fwd * pixels_per_meter};
      const auto bpx = static_cast<int>(center.x + along_right * pixels_per_meter);
      const auto bpy = static_cast<int>(center.y - along_fwd * pixels_per_meter);

      color = GRAY;
      if (ff.faction == Faction::ENEMY)
        color = RED;
      else if (ff.faction == Faction::FRIENDLY)
        color = GREEN;

      switch (typ.type) {
        case EntityType::AIRCRAFT:
          draw_aircraft(bpx, bpy, hd.heading + player_h, color);
          break;
        case EntityType::SAM:
        case EntityType::AAA:
          draw_sam(bpx, bpy, color);
          break;
        case EntityType::NAVAL:
          draw_ship(bpx, bpy, color);
          break;
        default:
        case EntityType::STRUCTURE:
        case EntityType::AIRBASE:
          draw_structure(bpx, bpy, color);
          break;
      }
      if (radar.locked_target == en) {
        const int alt_feet = static_cast<int>(meter_to_feet(pos.pos.y));
        DrawText(TextFormat("%d", alt_feet), bpx + 5, bpy - 5, 8, WHITE);
      }
    }
  }
}
