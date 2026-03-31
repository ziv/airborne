#include "DebugView.h"

void DebugView::draw(const Vector3 &position, const PilotControls &controls, const float speed) {
    const auto [x, y, z] = position;
    constexpr auto m = 100.0f;
    DrawRectangle(5, 5, 150, 200, BEIGE);
    DrawText(TextFormat("X: %0.f", x), 20, 10, 15, BLACK);
    DrawText(TextFormat("Z: %0.f", z), 20, 30, 15, BLACK);
    DrawText(TextFormat("Y: %0.f", y), 20, 50, 15, BLACK);
    DrawText(TextFormat("p: %f", controls.pitch * m), 20, 70, 15, BLUE);
    DrawText(TextFormat("r: %f", controls.roll * m), 20, 90, 15, BLUE);
    DrawText(TextFormat("y: %f", controls.yaw * m), 20, 110, 15, BLUE);
    DrawText(TextFormat("t: %f", controls.throttle), 20, 130, 15, BLUE);
    DrawText(TextFormat("s: %f", std::round(speed)), 20, 150, 15, BLUE);
    // DrawText(TextFormat("T: %f", controls.throttle * 100), 20, 170, 15, BLUE);
}
