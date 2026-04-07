#include "RadarView.h"
#include "raymath.h"
#include "../primitives/Constants.h"

void RadarView::update() {
    if (IsKeyPressed(KEY_R)) {
        rangeIndex = (rangeIndex + 1) % RANGE_COUNT;
    }
}

void RadarView::drawScope() const {
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

// void RadarView::drawContact(Vector2 blipPos, const RadarContact &contact) const {
//     DrawRectangle(blipPos.x - 3, blipPos.y - 3, 6, 6, contact.color);
//
//     int altFeet = static_cast<int>(contact.worldPosition.y * GamePhysics::METERS_TO_FEET);
//     DrawText(TextFormat("%d", altFeet), blipPos.x + 5, blipPos.y - 5, 10, contact.color);
// }

void RadarView::draw(const AircraftState &state,
                     const std::vector<RadarContact> &contacts) const {
    drawScope();

    const Meter range = RANGES[rangeIndex];
    const float pixelsPerMeter = displayRadius / range;

    // player absolute world position (accounting for large-world offset)
    const float playerX = state.position.x + state.mapOffset.x;
    const float playerZ = state.position.z + state.mapOffset.y;

    // project orientation onto XZ plane for top-down radar
    const Vector2 fwd = Vector2Normalize({
        state.orientation.forward.x,
        state.orientation.forward.z
    });
    const Vector2 right = Vector2Normalize({
        state.orientation.right.x,
        state.orientation.right.z
    });

    for (const auto &contact: contacts) {
        const float dx = contact.worldPosition.x - playerX;
        const float dz = contact.worldPosition.z - playerZ;

        // project onto player-relative heading frame
        const float alongFwd = dx * fwd.x + dz * fwd.y;
        const float alongRight = dx * right.x + dz * right.y;

        if (alongFwd * alongFwd + alongRight * alongRight > range * range) continue;

        // forward → up on screen (−Y), right → +X on screen
        Vector2 blipPos = {
            center.x + alongRight * pixelsPerMeter,
            center.y - alongFwd * pixelsPerMeter
        };

        if (Vector2Distance(blipPos, center) > displayRadius) continue;

        const auto bpx = static_cast<int>(blipPos.x);
        const auto bpy = static_cast<int>(blipPos.y);

        // drawContact(blipPos, contact);
        DrawRectangle(bpx - 3, bpy - 3, 6, 6, contact.color);

        const int altFeet = static_cast<int>(contact.worldPosition.y * GamePhysics::METERS_TO_FEET);
        DrawText(TextFormat("%d", altFeet), bpx + 5, bpy - 5, 10, WHITE);
    }
}
