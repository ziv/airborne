module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"
#include "raygui.h"

export module GameOptions;

import Components;
import JsonConfig;
import Resources;
import Accessors;

// void save(const GameOptions& options) {
//   auto j = parse_json_file(resources::options_path);
//   j["tilt"] = options.tilt;
//   j["fov"] = options.fov;
//   save_json_to_file(j, resources::options_path);
// }

export namespace game_options {
void options(entt::registry& registry) {
  if (registry.ctx().get<GameState>().status == GameStatus::PAUSED) {
    auto& options = get_options(registry);

    // todo render options menu
    constexpr auto win = (Rectangle){300, 200, 600, 300};

    DrawRectangleRec(win, BLACK);
    DrawRectangleRoundedLines(win, 0.0f, 25.0f, YELLOW);

    DrawText("PAUSED", 500, 250, 40, YELLOW);

    float tilt = options.get_tilt();
    GuiSlider((Rectangle){380, 320, 300, 20}, "Tilt view", TextFormat("%.2f", tilt), &tilt, 0.0f, 1.0f);
    if (tilt != options.get_tilt()) {
      // options.tilt = tilt;
      // options.changed = true;
    }

    float fov = options.get_fov();
    GuiSlider((Rectangle){380, 350, 300, 20}, "FOV", TextFormat("%.0f", fov), &fov, 40.0f, 120.0f);
    if (fov != options.get_fov()) {
      // options.fov = fov;
      // options.changed = true;
    }

    // if (options.changed) {
    //   if (GuiButton((Rectangle){380, 410, 300, 20}, "SAVE")) {
    //     save(options);
    //   }
    // }
  }
}
}  // namespace game_options