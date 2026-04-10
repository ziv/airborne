/**
 * @file RadarView.cpp
 * @brief Top-down radar scope rendering and contact plotting.
 */
#include "RadarView.h"
#include "raymath.h"
#include "CockpitViews.h"
#include "../../primitives/Constants.h"

RadarView::RadarView(GameData &gameData) : game(gameData) {
}

void RadarView::update() {
    if (IsKeyPressed(KEY_R)) {
        rangeIndex = (rangeIndex + 1) % RANGE_COUNT;
    }
}

void RadarView::drawScope(const Vector2 &center) const {
    const auto x = static_cast<int>(center.x);
    const auto y = static_cast<int>(center.y);
    const auto r = static_cast<int>(displayRadius);

    DrawCircleLines(x, y, displayRadius / 3.0f, DARKGREEN);
    DrawCircleLines(x, y, displayRadius * 2.0f / 3.0f, DARKGREEN);
    DrawCircleLines(x, y, displayRadius, DARKGREEN);

    // nose line
    DrawLine(x, y, x, y - r, {0, 100, 0, 255});

    const float rangeNm = RANGES[rangeIndex] * GamePhysics::METERS_TO_NM;
    DrawText(TextFormat("%.0f NM", rangeNm), x - r, y + r + 4, 10, DARKGREEN);
}

void RadarView::draw(const AircraftState &state,
                     const Vector2 &position) const {
    const Vector2 center = {position.x + Views::fwidth / 2, position.y + Views::fheight / 2};
    drawScope(center);

    const Meter range = RANGES[rangeIndex];
    const float rangeSq = range * range;
    const float pixelsPerMeter = displayRadius / range;
    auto color = GRAY;

    // player absolute world position (accounting for large-world offset)
    const float playerX = state.position.x - state.mapOffset.x;
    const float playerZ = state.position.z - state.mapOffset.y;

    // project orientation onto XZ plane for top-down radar
    const Vector2 fwd = Vector2Normalize({state.orientation.forward.x, state.orientation.forward.z});
    const Vector2 right = Vector2Normalize({state.orientation.right.x, state.orientation.right.z});

    // iterating items and if they are in range, display them on the radar
    game.entities.forEachAlive([&](const EntityBase &e) {
        const float dx = e.position.x - playerX;
        const float dz = e.position.z - playerZ;

        // project onto player-relative heading frame (dot product)
        const float alongFwd = dx * fwd.x + dz * fwd.y;
        const float alongRight = dx * right.x + dz * right.y;

        if (alongFwd * alongFwd + alongRight * alongRight > rangeSq) return;

        Vector2 blipPos = {center.x + alongRight * pixelsPerMeter, center.y - alongFwd * pixelsPerMeter};
        const auto bpx = static_cast<int>(blipPos.x);
        const auto bpy = static_cast<int>(blipPos.y);

        color = GRAY;
        if (e.faction == Faction::ENEMY) color = RED;
        else if (e.faction == Faction::FRIENDLY) color = GREEN;

        switch (e.type) {
            case EntityType::AIRCRAFT:
                drawAircraft(bpx, bpy, e.heading, color);
                break;
            case EntityType::SAM:
            case EntityType::AAA:
                drawSam(bpx, bpy, color);
                break;
            case EntityType::NAVAL:
                drawShip(bpx, bpy, color);
                break;
            default:
            case EntityType::STRUCTURE:
            case EntityType::AIRBASE:
                drawStructure(bpx, bpy, color);
                break;
        }
        const int altFeet = static_cast<int>(e.position.y * GamePhysics::METERS_TO_FEET);
        DrawText(TextFormat("%d", altFeet), bpx + 5, bpy - 5, 10, WHITE);
    });
}

/// @brief Ship blimp is a sircle
void RadarView::drawShip(const int x, const int y, const Color &color) {
    DrawCircle(x, y, 5, color);
}

/// @brief Aircraft blip is a triangle
void RadarView::drawAircraft(const int x, const int y, const float heading, const Color &color) {
    const auto fx = static_cast<float>(x);
    const auto fy = static_cast<float>(y);
    const auto rad = heading * DEG2RAD;
    const Vector2 fwd = {sinf(rad), -cosf(rad)};
    const Vector2 right = {fwd.y, -fwd.x};

    const Vector2 p1 = {fx + fwd.x * 6, fy + fwd.y * 6};
    const Vector2 p2 = {fx - fwd.x * 4 + right.x * 3, fy - fwd.y * 4 + right.y * 3};
    const Vector2 p3 = {fx - fwd.x * 4 - right.x * 3, fy - fwd.y * 4 - right.y * 3};

    DrawTriangle(p1, p2, p3, color);
}

/// @brief SAM/AAA blip is an X
void RadarView::drawSam(const int x, const int y, const Color &color) {
    const auto fx = static_cast<float>(x);
    const auto fy = static_cast<float>(y);
    DrawLineEx({fx - 3, fy - 3}, {fx + 3, fy + 3}, 2, color);
    DrawLineEx({fx - 3, fy + 3}, {fx + 3, fy - 3}, 2, color);
}

/// @brief SAM/AAA blip is a rectangle
void RadarView::drawStructure(const int x, const int y, const Color &color) {
    DrawRectangle(x - 2, y - 2, 4, 4, color);
}
