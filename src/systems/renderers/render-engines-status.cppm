module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module RenderSystem:EngineStatus;

import Components;

void draw_engine(const PlayerInputs &inputs, const Vector2 &center) {
  constexpr float radius = 30.0f;
  constexpr float thickness = 2.0f;
  constexpr int segments = 180;

  constexpr float inner_radius = radius - thickness;
  constexpr float outer_radius = radius;

  // angle range (upper semicircle: 180 to 360)
  constexpr float start_angle = 180.0f;
  constexpr float end_green = 300.0f;
  constexpr float end_orange = 330.0f;
  constexpr float end_red = 360.0f;

  // 1. draw background (transparent)
  DrawRing(center, inner_radius, outer_radius, start_angle, end_green, segments, Fade(GREEN, 0.2f));
  DrawRing(center, inner_radius, outer_radius, end_green, end_orange, segments, Fade(ORANGE, 0.2f));
  DrawRing(center, inner_radius, outer_radius, end_orange, end_red, segments, Fade(RED, 0.2f));

  const float current_end_angle = start_angle + ((inputs.throttle / 1.2f) * 180.0f);

  if (current_end_angle > start_angle) {
    const float draw_green = std::min(current_end_angle, end_green);
    DrawRing(center, inner_radius, outer_radius, start_angle, draw_green, segments, Fade(GREEN, 0.9f));
  }

  if (current_end_angle > end_green) {
    const float draw_orange = std::min(current_end_angle, end_orange);
    DrawRing(center, inner_radius, outer_radius, end_green, draw_orange, segments, Fade(ORANGE, 0.9f));
  }

  if (current_end_angle > end_orange) {
    const float draw_red = std::min(current_end_angle, end_red);
    DrawRing(center, inner_radius, outer_radius, end_orange, draw_red, segments, Fade(RED, 0.9f));
  }
}

export void render_engine_status(entt::registry &registry) {
  const auto view = registry.view<DashboardSlot, EngineWidget, Position2D>();

  for (auto [entity, slot, pos] : view.each()) {
    const auto player_entity = registry.ctx().get<PlayerEntity>().id;
    const auto [player, inputs] = registry.get<Player, PlayerInputs>(player_entity);

    // const Vector2 center = {pos.pos.x + Views::fwidth / 2, pos.pos.y + Views::fheight / 2};
    const auto ix = static_cast<int>(pos.pos.x);
    const auto iy = static_cast<int>(pos.pos.y);

    DrawText("ENGINES", ix + 10, iy + 10, 10, Fade(WHITE, 0.3));

    const auto v1 = (Vector2){pos.pos.x + 45.0f, pos.pos.y + 60};
    const auto v2 = (Vector2){pos.pos.x + 115.0f, pos.pos.y + 60};

    draw_engine(inputs, v1);
    draw_engine(inputs, v2);

    const auto t = static_cast<int>(floor(inputs.throttle * 100));

    // to keep the text in the middle of the gauge
    const auto padding = t > 99 ? 14 : t > 9 ? 10 : 6;
    auto line = iy + 90;

    DrawText(TextFormat("%d%%", t), static_cast<int>(v1.x) - padding, static_cast<int>(v1.y) - 8, 15, Fade(WHITE, 0.5));
    DrawText(TextFormat("%d%%", t), static_cast<int>(v2.x) - padding, static_cast<int>(v2.y) - 8, 15, Fade(WHITE, 0.5));

    DrawText("BRAKES", ix + 10, line, 10, Fade(WHITE, 0.3));
    if (inputs.brakes)
      DrawText("ON", ix + 90, line, 10, Fade(WHITE, 0.9f));
    else
      DrawText("OFF", ix + 90, line, 10, Fade(WHITE, 0.3f));

    line += 20;

    DrawText("GEAR", ix + 10, line, 10, Fade(WHITE, 0.3));
    if (inputs.gear)
      DrawText("OUT", ix + 90, line, 10, Fade(WHITE, 0.9f));
    else
      DrawText("IN", ix + 90, line, 10, Fade(WHITE, 0.3f));

    line += 20;

    DrawText("FUEL", ix + 10, line, 10, Fade(WHITE, 0.3f));
  }
}
