#include "HudView.h"

#include "../primitives/Utils.h"

void HudView::update(const GameData &game) {
    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_H)) {
        current++;
        if (current > 2) current = 0;
    }
}

void HudView::draw(const GameData &game) const {
    const Color color = colrs[current];
    constexpr int HudWidth = 300;
    constexpr int HudHeight = 230;
    const int HudX = (GetScreenWidth() - HudWidth) / 2;
    const int HudY = (GetScreenHeight() - HudHeight) / 2 - 170;

    BeginScissorMode(HudX, HudY, HudWidth, HudHeight);
    // ClearBackground(ORANGE);
    const Camera rayCam = game.getCamera();
    const Vector3 camForward = Vector3Normalize(Vector3Subtract(rayCam.target, game.getPosition()));
    const Vector3 forward = game.getForward();
    const Vector3 up = game.getUp();

    // --- heading projected to horizontal plane ---
    const Vector3 flatForward = GetFlatForward(forward, up);
    const Vector3 flatRight = GetFlatRight(flatForward);

    // --- screen-space sky reference (computed once, valid for all rungs) ---
    const auto width = GetScreenWidth();
    const auto height = GetScreenHeight();
    const Vector3 refPt = Vector3Add(game.getPosition(), Vector3Scale(camForward, 10000.0f));
    const Vector3 skyPt = Vector3Add(refPt, Vector3Scale(GamePhysics::WorldUp, 500.0f));
    const Vector2 refScr = GetWorldToScreenEx(refPt, rayCam, width, height);
    const Vector2 skyScr = GetWorldToScreenEx(skyPt, rayCam, width, height);

    Vector2 skyRef = {skyScr.x - refScr.x, skyScr.y - refScr.y};
    if (const float skyRefLen = sqrtf(skyRef.x * skyRef.x + skyRef.y * skyRef.y); skyRefLen > 0.001f) {
        skyRef.x /= skyRefLen;
        skyRef.y /= skyRefLen;
    }


    // --- speed & altitude labels (relative to HUD rect) ---
    DrawText(TextFormat("%0.f", std::round(game.speed * 3.6)), HudX + 10, HudY + HudHeight / 2, 15, color);
    // DrawText(TextFormat("%0.f", FormatNumber(std::round(game.GetPosition().y)).c_str()), HudX + HudSize - 30, HudY + HudSize / 2, 15, GREEN);
    DrawText(FormatNumber(std::round(game.getPosition().y)).c_str(), HudX + HudWidth - 40, HudY + HudHeight / 2, 15,
             color);

    // --- pitch ladder ---
    for (int angle = -80; angle <= 80; angle += 20) {
        const float pitchRad = static_cast<float>(angle) * PI / 180.0f;
        const Vector3 rungDir = Vector3RotateByAxisAngle(flatForward, flatRight, pitchRad);

        // check if behind camera
        if (Vector3DotProduct(rungDir, camForward) < 0.1f) continue;

        // project rung center AND one offset point to get screen-space direction
        const Vector3 center3D = Vector3Add(game.getPosition(), Vector3Scale(rungDir, 10000.0f));
        const Vector3 side3D = Vector3Add(center3D, Vector3Scale(flatRight, 500.0f));

        const Vector2 center =
                GetWorldToScreenEx(center3D, rayCam, GetScreenWidth(), GetScreenHeight());
        const Vector2 side = GetWorldToScreenEx(side3D, rayCam, GetScreenWidth(), GetScreenHeight());

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
        DrawLineEx(start, gapL, thick, color);
        DrawLineEx(gapR, end, thick, color);

        if (angle == 0) {
            // horizon line — ticks point down (toward ground)
            DrawLineEx(start, {
                           start.x - perp.x * tick,
                           start.y - perp.y * tick
                       }, thick, color);
            DrawLineEx(end, {
                           end.x - perp.x * tick,
                           end.y - perp.y * tick
                       }, thick, color);
        } else {
            // ticks always point toward the horizon (0°)
            const float sign = (angle > 0) ? -1.0f : 1.0f;
            DrawLineEx(start, {
                           start.x + perp.x * sign * tick,
                           start.y + perp.y * sign * tick
                       }, thick, color);
            DrawLineEx(end, {
                           end.x + perp.x * sign * tick,
                           end.y + perp.y * sign * tick
                       }, thick, color);

            DrawText(TextFormat("%d", angle),
                     static_cast<int>(start.x) - 20,
                     static_cast<int>(start.y) - 5, 10, color);
        }
    }
    EndScissorMode();
}
