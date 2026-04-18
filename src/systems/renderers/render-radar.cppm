module;
#include <entt/entt.hpp>
#include "../../lib/ray.hpp"

export module RenderSystem:Radar;

import Components;
import Helpers;
import Accessors;
import Types;

void drawScope(const Vector2 &center, const float displayRadius, const float range) {
    const auto x = static_cast<int>(center.x);
    const auto y = static_cast<int>(center.y);
    const auto r = static_cast<int>(displayRadius);

    DrawCircleLines(x, y, displayRadius / 3.0f, DARKGREEN);
    DrawCircleLines(x, y, displayRadius * 2.0f / 3.0f, DARKGREEN);
    DrawCircleLines(x, y, displayRadius, DARKGREEN);

    // nose line
    DrawLine(x, y, x, y - r, {0, 100, 0, 255});

    const float rangeNm = meter_to_nm(range);
    DrawText(TextFormat("%.0f NM", rangeNm), x - r - 20, y + r + 4, 10, DARKGREEN);
}

/// @brief Aircraft blip is a triangle
void drawAircraft(int x, int y, float heading, const Color &color) {
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

/// @brief Ship blip is a sircle
void drawShip(int x, int y, const Color &color) {
    DrawCircle(x, y, 5, color);
}

/// @brief SAM/AAA blip is an X
void drawSam(int x, int y, const Color &color) {
    const auto fx = static_cast<float>(x);
    const auto fy = static_cast<float>(y);
    DrawLineEx({fx - 3, fy - 3}, {fx + 3, fy + 3}, 2, color);
    DrawLineEx({fx - 3, fy + 3}, {fx + 3, fy - 3}, 2, color);
}

/// @brief SAM/AAA blip is a rectangle
void drawStructure(int x, int y, const Color &color) {
    DrawRectangle(x - 2, y - 2, 4, 4, color);
}

export void RenderRadar(entt::registry &registry) {
    const auto view = registry.view<DashboardSlot, RadarWidget, Position2D>();
    if (view.begin() == view.end()) return;

    const entt::entity entity = view.front();
    const auto [wd, pos] = registry.get<RadarWidget, Position2D>(entity);
    const auto &player = get_player(registry);

    const auto displayRadius = wd.cfg.size / 2.0f;
    const Vector2 center = {pos.pos.x + displayRadius, pos.pos.y + displayRadius};
    const float range = wd.cfg.ranges[wd.rangeIndex];

    drawScope(center, displayRadius, range);


    const float rangeSq = range * range;
    const float pixelsPerMeter = displayRadius / range;
    auto color = GRAY;

    // player absolute world position (accounting for large-world offset)
    const float playerX = player.pos.x - player.offset.x;
    const float playerZ = player.pos.z - player.offset.y;

    // project orientation onto XZ plane for top-down radar
    const Vector2 fwd = Vector2Normalize({player.forward.x, player.forward.z});
    const Vector2 right = Vector2Normalize({player.right.x, player.right.z});

    // iterating items and if they are in range, display them on the radar
    const auto blip = registry.view<Identify, Position3D, FriendFoe, IdentifyType>();
    for (auto [ent, id, position, faction, type]: blip.each()) {
        const float dx = position.pos.x - playerX;
        const float dz = position.pos.z - playerZ;

        // project onto player-relative heading frame (dot product)
        const float alongFwd = dx * fwd.x + dz * fwd.y;
        const float alongRight = dx * right.x + dz * right.y;

        if (alongFwd * alongFwd + alongRight * alongRight > rangeSq) return;

        Vector2 blipPos = {center.x + alongRight * pixelsPerMeter, center.y - alongFwd * pixelsPerMeter};
        const auto bpx = static_cast<int>(blipPos.x);
        const auto bpy = static_cast<int>(blipPos.y);

        color = GRAY;
        if (faction.faction == Faction::ENEMY) color = RED;
        else if (faction.faction == Faction::FRIENDLY) color = GREEN;

        switch (type.type) {
            case EntityType::AIRCRAFT:
                drawAircraft(bpx, bpy, 0, color);
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
        // const int altFeet = static_cast<int>(e.position.y * GamePhysics::METERS_TO_FEET);
        // DrawText(TextFormat("%d", altFeet), bpx + 5, bpy - 5, 10, WHITE);
    }
}
