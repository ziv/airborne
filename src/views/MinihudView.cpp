#include "MinihudView.h"
#include "raymath.h"
#include "rlgl.h"

MinihudView::MinihudView(const AppConfig &config) : ladderX(config.get<int>("/views/hudLadderX")),
                                                    ladderY(config.get<int>("/views/hudLadderY")),
                                                    ladderWidth(config.get<int>("/views/hudLadderWidth")),
                                                    ladderHeight(config.get<int>("/views/hudLadderHeight")),
                                                    fov(config.get<float>("/pilot/fov")),
                                                    width(config.get<int>("/config/screenWidth")),
                                                    height(config.get<int>("/config/screenHeight")),
                                                    ladderOffset(config.get<int>("/views/hudLadderOffset")) {
}

void MinihudView::update() {
    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_H)) {
        color++;
        if (color > 2) color = 0;
    }
}

void MinihudView::draw(const AircraftState &state) const {
    BeginScissorMode(ladderX, ladderY, ladderWidth, ladderHeight);

    const auto fy = state.orientation.forward.y;
    const auto uy = state.orientation.up.y;
    const auto [x, y, z] = state.orientation.right;

    const float pitch = asinf(fy) * RAD2DEG;
    const float roll = (fabsf(fy) < 0.999f ? atan2f(-y, uy) : atan2f(z, x)) * RAD2DEG;

    const auto centerX = width / 2;
    const auto centerY = height / 2;

    const float pixelsPerDegree = height / fov;

    rlPushMatrix();
    rlTranslatef(static_cast<float>(centerX), static_cast<float>(centerY) - ladderOffset, 0);
    rlRotatef(-roll, 0, 0, 1);
    rlTranslatef(0, pitch * pixelsPerDegree, 0);

    DrawLineEx({-100, 0}, {100, 0}, 2, colors[color]);
    for (int i = -180; i <= 180; i += 15) {
        if (i == 0) continue;
        const auto lineY = -static_cast<float>(i) * pixelsPerDegree;
        DrawLineEx({-100, lineY}, {100, lineY}, 1, colors[color]);
        auto to = i > 0 ? lineY + 10 : lineY - 10;
        DrawLineEx({100, lineY}, {110, to}, 1, colors[color]);
        DrawLineEx({-100, lineY}, {-110, to}, 1, colors[color]);
        DrawText(TextFormat("%d", i), -130, lineY - 5, 10, colors[color]);
        DrawText(TextFormat("%d", i), 115, lineY - 5, 10, colors[color]);
    }
    rlPopMatrix();
    EndScissorMode();

    // speed indicator
    DrawText(TextFormat("%d", static_cast<int>(state.forces.speed)), 430, 310, 15, colors[color]);
    // height indicator
    DrawText(TextFormat("%d", static_cast<int>(state.position.y - state.groundHeight)), 750, 310, 15, colors[color]);

    // DrawLine(centerX - 30, centerY - 120, centerX - 10, centerY - 120, RED);
    // DrawLine(centerX + 10, centerY - 120, centerX + 30, centerY - 120, RED);
    // DrawCircle(centerX, centerY - 120, 3, RED);

    if (state.controls.throttle > 1.0f) {
        // מדפיסים אזהרת מבער אחורי מהבהבת או בצבע שונה
        DrawText("A/B ON", 50, 470, 20, ORANGE);
    }
}
