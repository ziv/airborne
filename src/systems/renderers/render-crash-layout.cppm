module;
#include <entt/entt.hpp>
#include "../../lib/ray.hpp"

export module RenderSystem:CrashLayout;

import Accessors;

export void RenderCrashLayout(entt::registry &registry) {
    if (!is_player_crashed(registry)) return;

    const int sw = GetScreenWidth();
    const int sh = GetScreenHeight();
    // dim the screen
    DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.5f));
    // main message
    const auto msg = "CRASHED";
    const int msgW = MeasureText(msg, 40);
    DrawText(msg, sw / 2 - msgW / 2, sh / 2 - 30, 40, RED);
    // hint
    const auto hint = "Press SPACE to continue";
    const int hintW = MeasureText(hint, 20);
    DrawText(hint, sw / 2 - hintW / 2, sh / 2 + 20, 20, WHITE);
}
