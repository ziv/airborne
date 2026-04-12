//
// Created by Ziv Perry on 09/04/2026.
//

#include "EngineView.h"
#include <algorithm>
#include "CockpitViews.h"

void EngineView::drawEngine(const AircraftState &state, const Vector2 &center) const {
    constexpr float radius = 30.0f;
    constexpr float thickness = 2.0f;
    constexpr int segments = 180;

    constexpr float innerRadius = radius - thickness;
    constexpr float outerRadius = radius;

    // angle range (upper semicircle: 180 to 360)
    constexpr float startAngle = 180.0f;
    constexpr float endGreen = 300.0f;
    constexpr float endOrange = 330.0f;
    constexpr float endRed = 360.0f;

    // 1. draw background (transparent)
    DrawRing(center, innerRadius, outerRadius, startAngle, endGreen, segments, Fade(GREEN, 0.2f));
    DrawRing(center, innerRadius, outerRadius, endGreen, endOrange, segments, Fade(ORANGE, 0.2f));
    DrawRing(center, innerRadius, outerRadius, endOrange, endRed, segments, Fade(RED, 0.2f));

    const float currentEndAngle = startAngle + ((state.controls.throttle / 1.2f) * 180.0f);

    if (currentEndAngle > startAngle) {
        const float drawGreen = std::min(currentEndAngle, endGreen);
        DrawRing(center, innerRadius, outerRadius, startAngle, drawGreen, segments, Fade(GREEN, 0.9f));
    }

    if (currentEndAngle > endGreen) {
        const float drawOrange = std::min(currentEndAngle, endOrange);
        DrawRing(center, innerRadius, outerRadius, endGreen, drawOrange, segments, Fade(ORANGE, 0.9f));
    }

    if (currentEndAngle > endOrange) {
        const float drawRed = std::min(currentEndAngle, endRed);
        DrawRing(center, innerRadius, outerRadius, endOrange, drawRed, segments, Fade(RED, 0.9f));
    }

    // const float needleAngleRad = currentEndAngle * DEG2RAD;
    // const Vector2 needleStart = {
    //     center.x + std::cos(needleAngleRad) * innerRadius,
    //     center.y + std::sin(needleAngleRad) * innerRadius
    // };
    // const Vector2 needleEnd = {
    //     center.x + std::cos(needleAngleRad) * (outerRadius + 5.0f), // sticks out slightly
    //     center.y + std::sin(needleAngleRad) * (outerRadius + 5.0f)
    // };

    // DrawLineEx(needleStart, needleEnd, 2.0f, WHITE);
}

void EngineView::draw(const AircraftState &state, const Vector2 &position) const {
    const Vector2 center = {position.x + Views::fwidth / 2, position.y + Views::fheight / 2};
    const auto ix = static_cast<int>(position.x);
    const auto iy = static_cast<int>(position.y);


    DrawText("ENGINES", ix + 10, iy + 10, 10, Fade(WHITE, 0.3));

    const auto v1 = (Vector2){position.x + 45.0f, position.y + 60};
    const auto v2 = (Vector2){position.x + 115.0f, position.y + 60};

    drawEngine(state, v1);
    drawEngine(state, v2);

    const auto t = static_cast<int>(floor(state.controls.throttle * 100));

    // to keep the text in the middle of the gauge
    const auto padding = t > 99 ? 14 : t > 9 ? 10 : 6;
    auto line = iy + 90;

    DrawText(TextFormat("%d%%", t), static_cast<int>(v1.x) - padding, static_cast<int>(v1.y) - 8, 15, Fade(WHITE, 0.5));
    DrawText(TextFormat("%d%%", t), static_cast<int>(v2.x) - padding, static_cast<int>(v2.y) - 8, 15, Fade(WHITE, 0.5));

    DrawText("BRAKES", ix + 10, line, 10, Fade(WHITE, 0.3));
    if (state.controls.brakes) DrawText("ON", ix + 90, line, 10, Fade(WHITE, 0.9f));
    else DrawText("OFF", ix + 90, line, 10, Fade(WHITE, 0.3f));

    line += 20;

    DrawText("GEAR", ix + 10, line, 10, Fade(WHITE, 0.3));
    if (state.controls.gear) DrawText("OUT", ix + 90, line, 10, Fade(WHITE, 0.9f));
    else DrawText("IN", ix + 90, line, 10, Fade(WHITE, 0.3f));

    line += 20;

    DrawText("FUEL", ix  + 10,  line, 10, Fade(WHITE, 0.3f));
}
