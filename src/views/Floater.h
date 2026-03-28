#pragma  once
#include <memory>
#include "../primitives/GameData.h"

inline void DrawFloater(const GameData &game) {
    const auto [x, y, z] = game.GetCamera().position;
    constexpr auto m = 100.0f;
    DrawText(TextFormat("X: %0.f", x), 20, 40, 20, BLACK);
    DrawText(TextFormat("Z: %0.f", z), 20, 60, 20, BLACK);
    DrawText(TextFormat("Y: %0.f", y), 20, 80, 20, BLACK);
    DrawText(TextFormat("p: %f", game.controls.Pitch * m), 20, 100, 20, GREEN);
    DrawText(TextFormat("r: %f", game.controls.Roll * m), 20, 120, 20, GREEN);
    DrawText(TextFormat("y: %f", game.controls.Yaw * m), 20, 140, 20, GREEN);
    DrawText(TextFormat("t: %f", game.controls.Throttle * m), 20, 160, 20, GREEN);
    DrawText(TextFormat("s: %f", std::round(game.Speed())), 20, 180, 20, GREEN);
    DrawText(TextFormat("T: %f", game.throttle * 100), 20, 200, 20, GREEN);
}
