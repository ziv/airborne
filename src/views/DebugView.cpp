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
    constexpr auto m = 100.0f;
    DrawRectangle(5, 5, 150, 230, BEIGE);

    DrawText(TextFormat("Xr: %0.f", p.x), 20, 10, 15, BLACK);
    DrawText(TextFormat("Zr: %0.f", p.z), 20, 30, 15, BLACK);
    DrawText(TextFormat("Yr: %0.f", p.y), 20, 50, 15, BLACK);

    DrawText(TextFormat("Xa: %0.f", p.x - o.x), 20, 70, 15, BLUE);
    DrawText(TextFormat("Za: %0.f", p.z), 20, 90, 15, BLUE);
    DrawText(TextFormat("Ya: %0.f", p.y - o.y), 20, 110, 15, BLUE);



    // DrawText(TextFormat("Yr:%f", game.state.groundHeight), 20, 70, 15, BLACK);
    // DrawText(TextFormat("p: %f", c.pitch * m), 20, 90, 15, BLUE);
    // DrawText(TextFormat("r: %f", c.roll * m), 20, 110, 15, BLUE);
    // DrawText(TextFormat("y: %f", c.yaw * m), 20, 130, 15, BLUE);
    // DrawText(TextFormat("t: %f", c.throttle), 20, 150, 15, BLUE);
    // DrawText(TextFormat("s: %f", std::round(game.state.forces.speed)), 20, 170, 15, BLUE);
    // DrawText(TextFormat("G: %d", c.gear), 20, 190, 15, BLUE);
    // DrawText(TextFormat("B: %d", c.brakes), 20, 210, 15, BLUE);
}
