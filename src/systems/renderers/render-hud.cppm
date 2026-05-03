module;

#include <entt/entt.hpp>

#include "../../lib/ray.hpp"
#include "rlgl.h"
export module RenderSystem:Hud;

import Components;
import Helpers;
import Accessors;

void draw_ladder(const HudWidget &widget, const Player &player, const Color color) {
  const auto fy = player.forward.y;
  const auto uy = player.up.y;
  const auto [x, y, z] = player.right;

  const auto pitch = asinf(fy) * RAD2DEG;
  const auto roll = (fabsf(fy) < 0.999f ? atan2f(-y, uy) : atan2f(z, x)) * RAD2DEG;

  BeginScissorMode(widget.cfg.ladder.x, widget.cfg.ladder.y, widget.cfg.ladder.width, widget.cfg.ladder.height);
  DrawCircleLines(widget.center_x, widget.center_y - widget.cfg.ladder.offset, 5.0f, color);

  // freeze state
  rlPushMatrix();

  // take us to the center
  // the offset allow to define where to put the 0 line
  rlTranslatef(static_cast<float>(widget.center_x), static_cast<float>(widget.center_y) - static_cast<float>(widget.cfg.ladder.offset), 0);

  // pitch & roll
  rlRotatef(-roll, 0, 0, 1);
  rlTranslatef(0, pitch * widget.ppd, 0);

  // main line
  DrawLineEx({-100, 0}, {-20, 0}, 2, color);
  DrawLineEx({20, 0}, {100, 0}, 2, color);

  for (int i = -180; i <= 180; i += 15) {
    if (i == 0) continue;

    const auto line_y = -static_cast<float>(i * widget.pixels_per_degree);
    const auto line_yint = static_cast<int>(line_y);

    // main line
    DrawLineEx({-100, line_y}, {-20, line_y}, 1, color);
    DrawLineEx({20, line_y}, {100, line_y}, 1, color);

    // wings
    const auto to = i > 0 ? line_y + 10 : line_y - 10;
    DrawLineEx({100, line_y}, {110, to}, 1, color);
    DrawLineEx({-100, line_y}, {-110, to}, 1, color);
    DrawText(TextFormat("%d", i), -130, line_yint - 5, 10, color);
    DrawText(TextFormat("%d", i), 115, line_yint - 5, 10, color);
  }

  // resume from freeze
  rlPopMatrix();
  EndScissorMode();
}

void draw_warnings(const HudWidget &widget, const PlayerInputs &inputs, const Color color) {
  // after burner warning
  if (inputs.throttle > 1.0f) {
    DrawText("A/B ON", widget.cfg.warnings.x, widget.cfg.warnings.y, widget.cfg.warnings.font, ORANGE);
  }

  // autopilot warning
  if (inputs.autopilot) {
    DrawText("A/P ON", widget.cfg.warnings.x, widget.cfg.warnings.y, widget.cfg.warnings.font, color);
  }
}

void gear_warning(const HudWidget &widget, const Player &player, const PlayerInputs &inputs, const bool flying, const Color color) {
  if (flying && player.pos.y < 500.0f && !inputs.gear) {
    constexpr double blink_speed = 5.0;
    const auto alpha = static_cast<float>((std::sin(GetTime() * blink_speed) + 1.0) / 2.0);

    DrawText("GEAR UP", widget.cfg.warnings.x + 120, widget.cfg.warnings.y, widget.cfg.warnings.font, Fade(RED, alpha));
  }

  // if (!inputs.gear && player.pos.y < player.ground_height + 100.0f) {
  //   DrawText("GEAR UP", widget.cfg.gearWarning.x, widget.cfg.gearWarning.y, widget.cfg.gearWarning.font, RED);
  // }
}

void draw_heading(const HudWidget &widget, const Player &player, const Color color) {
  const auto [x, y, width, font] = widget.cfg.heading;
  const auto fx = static_cast<float>(x);
  // constexpr auto pixels_per_degree = 10.0f;
  constexpr auto tick_interval = 5;

  auto current_heading = std::atan2(-player.forward.x, player.forward.z) * RAD2DEG;
  // apply 180-degree offset to align north (0) with the world's 180 degree
  // mark. since atan2 returns [-180, 180], adding 180 shifts the range to [0,
  // 360].
  current_heading += 180.0f;

  // normalize 360 to 0
  if (current_heading >= 360.0f) {
    current_heading -= 360.0f;
  }

  DrawLine(x, y, x, y + font / 2, color);
  DrawText(TextFormat("%03.0f", current_heading), x - font / 2, y - font, font, color);

  const auto half_width = width / 2;
  const auto half_width_f = static_cast<float>(half_width);
  DrawLine(x - half_width, y, x + half_width, y, color);

  for (auto i = 0; i < 360; i += tick_interval) {
    // calculate shortest angular difference between current heading and the
    // tick
    auto diff = static_cast<float>(i) - current_heading;

    // normalize difference to [-180, 180] to handle the 360->0 wrap around
    if (diff > 180.0f)
      diff -= 360.0f;
    else if (diff < -180.0f)
      diff += 360.0f;

    // calculate screen X position and check if it's within the HUD tape width
    // using if-init
    if (const auto tick_x = fx + (diff * widget.ppd); tick_x >= fx - half_width_f && tick_x <= fx + half_width_f) {
      const auto is_major_tick = (i % 10 == 0);
      const auto tick_length = is_major_tick ? font / 2 : font / 4;

      // Draw the tick line
      DrawLine(static_cast<int>(tick_x), y, static_cast<int>(tick_x), y + tick_length, color);

      // Draw text for major ticks (every 10 degrees)
      if (is_major_tick) {
        const auto *text = TextFormat("%03d", i);
        const auto text_offset = static_cast<float>(font) * 0.6f;

        DrawText(text, static_cast<int>(tick_x - text_offset), y + tick_length + 4, static_cast<int>(static_cast<float>(font) * 0.8f), color);
      }
    }
  }
}

void draw_boresight(const HudWidget &widget, const Color color) {
  const auto [x, y, size] = widget.cfg.boresight;
  const auto left = x - size / 2;
  const auto right = x + size / 2;
  const auto part = size / 4;
  const auto tip = size / 8;

  const auto noseY = y - static_cast<int>(widget.tilt * RAD2DEG * widget.ppd);
  // let line
  DrawLine(left, noseY, left + part, noseY, color);
  // right line
  DrawLine(right - part, noseY, right, noseY, color);

  // W
  DrawLine(left + part, noseY, left + part + tip, noseY + tip, color);
  DrawLine(left + part + tip, noseY + tip, x, noseY, color);
  DrawLine(x, noseY, right - part - tip, noseY + tip, color);
  DrawLine(right - part - tip, noseY + tip, right - part, noseY, color);
}

void draw_height_indicator(const HudWidget &widget, const Player &player, const GroundHeight &gh, const Color color) {
  const auto height_absolute = meter_to_feet(player.pos.y);
  DrawText(TextFormat("%s", number_suffix(height_absolute)), widget.cfg.height.x, widget.cfg.height.y, widget.cfg.height.font, color);
  DrawRectangleLines(745, 328, 30, 14, color);
  const auto font = static_cast<int>(widget.cfg.height.font * 0.8f);
  const auto height = meter_to_feet(gh.height);
  const auto height_relative = height_absolute - height;
  DrawText(TextFormat("%s", number_suffix(height_relative)), widget.cfg.height.x + 2, widget.cfg.height.y + 19, font, color);
}

void draw_speed_indicator(const HudWidget &widget, const Player &player, Color color) {
  const auto speed = ms_to_knots(player.speed);
  DrawText(TextFormat("%s", number_suffix(speed)), widget.cfg.speedometer.x, widget.cfg.speedometer.y, widget.cfg.speedometer.font, color);
}

void draw_rate_of_climb(const HudWidget &widget, const Player &player, Color color) {
  const auto vertical_speed_fpm = ms_to_fpm(player.velocity.y);
  constexpr float MAX_CLIMB_RATE_FPM = 50000.0f;
  const float MAX_BAR_PIXELS = (static_cast<float>(widget.cfg.roc.height) / 2.0f) - 20.0f;

  float vs_ratio = vertical_speed_fpm / MAX_CLIMB_RATE_FPM;
  if (vs_ratio > 1.0f) vs_ratio = 1.0f;
  if (vs_ratio < -1.0f) vs_ratio = -1.0f;

  const int current_bar_height = static_cast<int>(vs_ratio * MAX_BAR_PIXELS);
  const int max_bar_pixels = static_cast<int>(MAX_BAR_PIXELS);

  const int center_x = widget.cfg.roc.x;  // see speed location
  const int center_y = widget.cfg.roc.y;
  const int width = widget.cfg.roc.width;
  DrawLine(center_x, center_y - max_bar_pixels, center_x, center_y + max_bar_pixels, Fade(color, 0.3f));
  DrawLine(center_x - width, center_y, center_x + width, center_y, color);

  if (current_bar_height > 0) {
    DrawRectangle(center_x - width / 2, center_y - current_bar_height, width, current_bar_height, color);
  } else {
    DrawRectangle(center_x - width / 2, center_y, width, -current_bar_height, color);
  }
}

export void render_hud(entt::registry &registry) {
  const auto view = registry.view<HudWidget>();
  if (view.begin() == view.end()) return;

  constexpr std::array<Color, 5> colors = {GREEN, YELLOW, ORANGE, WHITE, BLACK};
  const auto entity = view.front();
  const auto &hud = registry.get<HudWidget>(entity);

  const auto &player_entity = get_player_entity(registry);
  const auto &[player, inputs, gh] = registry.get<Player, PlayerInputs, GroundHeight>(player_entity);
  const auto flyting = is_player_flying(registry);

  const int safe_index = hud.color_index >= 0 && hud.color_index < static_cast<int>(colors.size()) ? hud.color_index : 0;
  const auto color = colors[safe_index];

  draw_ladder(hud, player, color);
  draw_rate_of_climb(hud, player, color);
  draw_speed_indicator(hud, player, color);
  draw_height_indicator(hud, player, gh, color);
  draw_boresight(hud, color);
  draw_heading(hud, player, color);
  draw_warnings(hud, inputs, color);
  gear_warning(hud, player, inputs, flyting, color);
}
