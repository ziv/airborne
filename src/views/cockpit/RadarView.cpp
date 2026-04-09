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
    const float playerX = state.position.x + state.mapOffset.x;
    const float playerZ = state.position.z + state.mapOffset.y;

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

        DrawRectangle(bpx - 2, bpy - 2, 4, 4, color);
        const int altFeet = static_cast<int>(e.position.y * GamePhysics::METERS_TO_FEET);
        DrawText(TextFormat("%d", altFeet), bpx + 5, bpy - 5, 10, WHITE);
    });
}
