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
    Color ambient_value = vector_to_color(options.get_ambient_color());

    const Color prev_zenith_value = zenith_value;
    const Color prev_horizon_value = horizon_value;
    const Color prev_ambient_value = ambient_value;

    GuiWindowBox((Rectangle){256, 64, 440, 440}, "GAME PAUSED");
    GuiLabel((Rectangle){280, 104, 120, 24}, "Player view");
    GuiSlider((Rectangle){328, 144, 192, 16}, "tilt", TextFormat("%.2f", tilt_value), &tilt_value, 0, 1.0f);
    GuiSlider((Rectangle){328, 184, 192, 16}, "fovy", TextFormat("%.2f", fovy_value), &fovy_value, 45.0f, 90.0f);

    GuiColorPicker((Rectangle){328, 232, 96, 96}, "zenith", &zenith_value);
    GuiLabel((Rectangle){280, 232, 48, 24}, "zenith");

    GuiColorPicker((Rectangle){328, 352, 96, 96}, "horizon", &horizon_value);
    GuiLabel((Rectangle){280, 352, 48, 24}, "horizon");

    GuiColorPicker((Rectangle){528, 232, 96, 96}, "ambient", &ambient_value);
    GuiLabel((Rectangle){480, 232, 48, 24}, "ambient");

    if (GuiButton((Rectangle){328, 472, 96, 32}, "Save")) {
      options.save();
    }

    if (tilt_value != options.get_tilt()) {
      options.set_tilt(tilt_value);
    }
    if (fovy_value != options.get_fov()) {
      options.set_fov(fovy_value);
    }
    if (!compare_colors(zenith_value, prev_zenith_value)) {
      options.set_zenith_color(color_to_vector(zenith_value));
    }
    if (!compare_colors(horizon_value, prev_horizon_value)) {
      options.set_horizon_color(color_to_vector(horizon_value));
    }
    if (!compare_colors(ambient_value, prev_ambient_value)) {
      options.set_ambient_color(color_to_vector(ambient_value));
    }
  }
}
}  // namespace game_options