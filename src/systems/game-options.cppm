module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"
#include "raygui.h"

export module GameOptions;

import Components;
import JsonConfig;
import Resources;
import Accessors;

Color vector_to_color(const Vector3& vec) {
  return Color{static_cast<unsigned char>(vec.x * 255), static_cast<unsigned char>(vec.y * 255), static_cast<unsigned char>(vec.z * 255), 255};
}

Vector3 color_to_vector(const Color& color) {
  return Vector3{static_cast<float>(color.r) / 255.0f, static_cast<float>(color.g) / 255.0f, static_cast<float>(color.b) / 255.0f};
}

bool compare_colors(const Color& c1, const Color& c2) { return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a; }

export namespace game_options {
void options(entt::registry& registry) {
  if (registry.ctx().get<GameState>().status == GameStatus::PAUSED) {
    auto& options = get_options(registry);

    float tilt_value = options.get_tilt();
    float fovy_value = options.get_fov();

    Color zenith_value = vector_to_color(options.get_zenith_color());
    Color horizon_value = vector_to_color(options.get_horizon_color());

    const Color prevZenithValue = zenith_value;
    const Color prevHorizonValue = horizon_value;

    GuiWindowBox((Rectangle){256, 64, 440, 440}, "GAME PAUSED");
    GuiLabel((Rectangle){280, 104, 120, 24}, "Player view");
    GuiSlider((Rectangle){328, 144, 192, 16}, "tilt", TextFormat("%.2f", tilt_value), &tilt_value, 0, 1.0f);
    GuiSlider((Rectangle){328, 184, 192, 16}, "fovy", TextFormat("%.2f", fovy_value), &fovy_value, 45.0f, 90.0f);
    GuiColorPicker((Rectangle){328, 232, 96, 96}, "zenith", &zenith_value);
    GuiColorPicker((Rectangle){328, 352, 96, 96},"horizon",&horizon_value);
    GuiLabel((Rectangle){280, 232, 48, 24}, "zenith");
    GuiLabel((Rectangle){280, 352, 48, 24}, "horizon");

    if (tilt_value != options.get_tilt()) {
      options.set_tilt(tilt_value);
    }
    if (fovy_value != options.get_fov()) {
      options.set_fov(fovy_value);
    }
    if (!compare_colors(zenith_value, prevZenithValue)) {
      options.set_zenith_color(color_to_vector(zenith_value));
    }
    if (!compare_colors(horizon_value, prevHorizonValue)) {
      options.set_horizon_color(color_to_vector(horizon_value));
    }

    // ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    // DrawText("PAUSED", 500, 250, 40, YELLOW);

    // float tilt = options.get_tilt();
    // GuiSlider((Rectangle){380, 320, 300, 20}, "Tilt view", TextFormat("%.2f", tilt), &tilt, 0.0f, 1.0f);
    // if (tilt != options.get_tilt()) {
    //   // options.tilt = tilt;
    //   // options.changed = true;
    // }
    //
    // float fov = options.get_fov();
    // GuiSlider((Rectangle){380, 350, 300, 20}, "FOV", TextFormat("%.0f", fov), &fov, 40.0f, 120.0f);
    // if (fov != options.get_fov()) {
    //   // options.fov = fov;
    //   // options.changed = true;
    // }

    // if (options.changed) {
    //   if (GuiButton((Rectangle){380, 410, 300, 20}, "SAVE")) {
    //     save(options);
    //   }
    // }
  }
}
}  // namespace game_options