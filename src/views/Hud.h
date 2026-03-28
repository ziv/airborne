#pragma once
#include "../Constants.h"
#include "../primitives/GameData.h"

constexpr int HudSize = 280;
constexpr int HudX = (GameConfig::SCREEN_WIDTH - HudSize) / 2;
constexpr int HudY = (GameConfig::SCREEN_HEIGHT - HudSize) / 2; // - 100;

inline void DrawHud(const GameData &game) {
    BeginScissorMode(HudX, HudY, HudSize, HudSize);

    const Camera rayCam = game.GetCamera();
    const Vector3 camForward = Vector3Normalize(Vector3Subtract(rayCam.target, game.GetPosition()));
    const Vector3 forward = game.GetForward();
    const Vector3 up = game.GetUp();

    // --- heading projected to horizontal plane ---
    const Vector3 flatForward = GetFlatForward(forward, up);
    const Vector3 flatRight = GetFlatRight(flatForward);

    // --- screen-space sky reference (computed once, valid for all rungs) ---
    const Vector3 refPt = Vector3Add(game.GetPosition(), Vector3Scale(camForward, 10000.0f));
    const Vector3 skyPt = Vector3Add(refPt, Vector3Scale(GamePhysics::WorldUp, 500.0f));
    const Vector2 refScr = GetWorldToScreenEx(refPt, rayCam, GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT);
    const Vector2 skyScr = GetWorldToScreenEx(skyPt, rayCam, GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT);

    Vector2 skyRef = {skyScr.x - refScr.x, skyScr.y - refScr.y};
    if (const float skyRefLen = sqrtf(skyRef.x * skyRef.x + skyRef.y * skyRef.y); skyRefLen > 0.001f) {
        skyRef.x /= skyRefLen;
        skyRef.y /= skyRefLen;
    }


    // --- speed & altitude labels (relative to HUD rect) ---
    DrawText(TextFormat("%0.f", std::abs(game.Speed())), HudX + 10, HudY + HudSize / 2, 15, GREEN);
    DrawText(TextFormat("%0.f", std::abs(game.GetPosition().y)), HudX + HudSize - 30, HudY + HudSize / 2, 15, GREEN);

    // --- pitch ladder ---
    for (int angle = -80; angle <= 80; angle += 20) {
        const float pitchRad = static_cast<float>(angle) * PI / 180.0f;
        const Vector3 rungDir = Vector3RotateByAxisAngle(flatForward, flatRight, pitchRad);

        // check if behind camera
        if (Vector3DotProduct(rungDir, camForward) < 0.1f) continue;

        // project rung center AND one offset point to get screen-space direction
        const Vector3 center3D = Vector3Add(game.GetPosition(), Vector3Scale(rungDir, 10000.0f));
        const Vector3 side3D = Vector3Add(center3D, Vector3Scale(flatRight, 500.0f));

        const Vector2 center =
                GetWorldToScreenEx(center3D, rayCam, GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT);
        const Vector2 side = GetWorldToScreenEx(side3D, rayCam, GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT);

        // screen-space rung direction
        Vector2 rd = {side.x - center.x, side.y - center.y};
        const float rdLen = sqrtf(rd.x * rd.x + rd.y * rd.y);
        if (rdLen < 0.001f) continue;
        rd.x /= rdLen;
        rd.y /= rdLen;

        // perpendicular — orient toward sky using the pre-computed reference
        Vector2 perp = {-rd.y, rd.x};
        if (perp.x * skyRef.x + perp.y * skyRef.y < 0.0f) {
            perp.x = -perp.x;
            perp.y = -perp.y;
        }
        // `perp` now points toward sky on screen

        const float halfLen = (angle == 0) ? 110.0f : 90.0f;
        constexpr float gapHalf = 20.0f;
        const float thick = (angle == 0) ? 3.0f : 2.0f;
        constexpr float tick = 15.0f;

        const Vector2 start = {
            center.x - rd.x * halfLen,
            center.y - rd.y * halfLen
        };
        const Vector2 end = {
            center.x + rd.x * halfLen,
            center.y + rd.y * halfLen
        };
        const Vector2 gapL = {
            center.x - rd.x * gapHalf,
            center.y - rd.y * gapHalf
        };
        const Vector2 gapR = {
            center.x + rd.x * gapHalf,
            center.y + rd.y * gapHalf
        };

        // the two halves of the rung (gap in the middle)
        DrawLineEx(start, gapL, thick, GREEN);
        DrawLineEx(gapR, end, thick, GREEN);

        if (angle == 0) {
            // horizon line — ticks point down (toward ground)
            DrawLineEx(start, {
                           start.x - perp.x * tick,
                           start.y - perp.y * tick
                       }, thick, GREEN);
            DrawLineEx(end, {
                           end.x - perp.x * tick,
                           end.y - perp.y * tick
                       }, thick, GREEN);
        } else {
            // ticks always point toward the horizon (0°)
            const float sign = (angle > 0) ? -1.0f : 1.0f;
            DrawLineEx(start, {
                           start.x + perp.x * sign * tick,
                           start.y + perp.y * sign * tick
                       }, thick, GREEN);
            DrawLineEx(end, {
                           end.x + perp.x * sign * tick,
                           end.y + perp.y * sign * tick
                       }, thick, GREEN);

            DrawText(TextFormat("%d", angle),
                     static_cast<int>(start.x) - 20,
                     static_cast<int>(start.y) - 5, 10, GREEN);
        }
    }
    EndScissorMode();
}
