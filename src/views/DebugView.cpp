#include "DebugView.h"

DebugView::DebugView(GameData &game) : game(game) {
}

void DebugView::update() {
    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_D)) display = !display;
}

void DebugView::draw() const {
    const auto [x, y, z] = game.aircraftCamera.getCamera().position;
    const auto c = game.aircraftControls.getControls();
    constexpr auto m = 100.0f;
    DrawRectangle(5, 5, 150, 200, BEIGE);
    DrawText(TextFormat("X: %0.f", x), 20, 10, 15, BLACK);
    DrawText(TextFormat("Z: %0.f", z), 20, 30, 15, BLACK);
    DrawText(TextFormat("Y: %0.f", y), 20, 50, 15, BLACK);
    DrawText(TextFormat("p: %f", c.pitch * m), 20, 70, 15, BLUE);
    DrawText(TextFormat("r: %f", c.roll * m), 20, 90, 15, BLUE);
    DrawText(TextFormat("y: %f", c.yaw * m), 20, 110, 15, BLUE);
    DrawText(TextFormat("t: %f", c.throttle), 20, 130, 15, BLUE);
    DrawText(TextFormat("s: %f", std::round(game.aircraftPhysics.getSpeed())), 20, 150, 15, BLUE);
    // DrawText(TextFormat("T: %f", controls.throttle * 100), 20, 170, 15, BLUE);
}
