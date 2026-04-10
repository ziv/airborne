/**
 * @file DebugView.cpp
 * @brief Debug overlay rendering — relative and absolute aircraft coordinates.
 */
#include "DebugView.h"

DebugView::DebugView(GameData &game) : game(game) {
}

void DebugView::update() {
    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_D)) display = !display;
}

void DebugView::draw() const {
    if (!display) return;
    const auto p = game.state.position;
    const auto o = game.state.mapOffset;
    const auto c = game.state.controls;
    const auto s = game.state.pos();
    constexpr auto m = 100.0f;
    int y = 10;
    const int margin = 20;
    DrawRectangle(5, 5, 150, 230, BEIGE);

    DrawText(TextFormat("Xr: %0.00f", p.x), margin, y, 15, BLACK);
    y += margin;
    DrawText(TextFormat("Zr: %0.00f", p.z), margin, y, 15, BLACK);
    y += margin;
    DrawText(TextFormat("Yr: %0.00f", p.y), margin, y, 15, BLACK);
    y += margin;
    DrawText(TextFormat("Xa: %0.00f, %0.00f", p.x - o.x, s.x), margin, y, 15, BLUE);
    y += margin;
    DrawText(TextFormat("Za: %0.00f, %0.00f", p.z - o.y, s.z), margin, y, 15, BLUE);
    y += margin;
    DrawText(TextFormat("Ya: %0.00f, %0.00f", p.y, s.y), margin, y, 15, BLUE);
    y += margin;
    DrawText(TextFormat("Yr:%f", game.state.groundHeight), margin, y, 15, BLACK);
    y += margin;
    DrawText(TextFormat("p: %f", c.pitch * m), margin, y, 15, BLUE);
    y += margin;
    DrawText(TextFormat("r: %f", c.roll * m), margin, y, 15, BLUE);
    y += margin;
    DrawText(TextFormat("t: %f", c.throttle), margin, y, 15, BLUE);
    y += margin;
    DrawText(TextFormat("s: %f", std::round(game.state.forces.speed)), margin, y, 15, BLUE);
    // DrawText(TextFormat("y: %f", c.yaw * m), 20, 130, 15, BLUE);
    // DrawText(TextFormat("G: %d", c.gear), 20, 190, 15, BLUE);
    // DrawText(TextFormat("B: %d", c.brakes), 20, 210, 15, BLUE);
}
