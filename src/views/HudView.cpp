#include "HudView.h"
#include "raylib.h"
#include "../primitives/Utils.h"

HudView::HudView(AppConfig &config) {
}

void HudView::update(const GameData &game) {
    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_H)) {
        colorIndex++;
        if (colorIndex > 2) colorIndex = 0;
    }
}

void HudView::draw(const Camera &camera, const AircraftState &state) {
    const Color c = colors[colorIndex];
    constexpr int hudWidth = 300;
    constexpr int hudHeight = 230;
    const auto width = GetScreenWidth();
    const auto height = GetScreenHeight();
    const int hudX = (width - hudWidth) / 2;
    const int hudY = (height - hudHeight) / 2 - 80;

    BeginScissorMode(hudX, hudY, hudWidth, hudHeight);


    // ClearBackground(ORANGE);
    auto rayCam = camera;
    const Vector3 camForward = Vector3Normalize(Vector3Subtract(rayCam.target, rayCam.position));
    // const Vector3 forward = directions.forward;
    // const Vector3 up = directions.up;

    // --- heading projected to horizontal plane ---
    const Vector3 flatForward = GetFlatForward(state.orientation.forward, state.orientation.up);
    const Vector3 flatRight = GetFlatRight(flatForward);

    // --- screen-space sky reference (computed once, valid for all rungs) ---
    const Vector3 refPt = Vector3Add(rayCam.position, Vector3Scale(camForward, 10000.0f));
    const Vector3 skyPt = Vector3Add(refPt, Vector3Scale(GamePhysics::WorldUp, 500.0f));
    const Vector2 refScr = GetWorldToScreenEx(refPt, rayCam, width, height);
    const Vector2 skyScr = GetWorldToScreenEx(skyPt, rayCam, width, height);

    Vector2 skyRef = {skyScr.x - refScr.x, skyScr.y - refScr.y};
    if (const float skyRefLen = sqrtf(skyRef.x * skyRef.x + skyRef.y * skyRef.y); skyRefLen > 0.001f) {
        skyRef.x /= skyRefLen;
        skyRef.y /= skyRefLen;
    }


    // --- speed & altitude labels (relative to HUD rect) ---
    DrawText(TextFormat("%0.f", std::round(state.forces.speed * 3.6)), hudX + 10, hudY + hudHeight / 2, 15, c);
    // DrawText(TextFormat("%0.f", FormatNumber(std::round(game.GetPosition().y)).c_str()), HudX + HudSize - 30, HudY + HudSize / 2, 15, GREEN);
    DrawText(FormatNumber(std::round(camera.position.y)).c_str(), hudX + hudWidth - 40, hudY + hudHeight / 2, 15, c);

    // --- pitch ladder ---
    for (int angle = -80; angle <= 80; angle += 20) {
        const float pitchRad = static_cast<float>(angle) * PI / 180.0f;
        const Vector3 rungDir = Vector3RotateByAxisAngle(flatForward, flatRight, pitchRad);

        // check if behind camera
        if (Vector3DotProduct(rungDir, camForward) < 0.1f) continue;

        // project rung center AND one offset point to get screen-space direction
        const Vector3 center3D = Vector3Add(camera.position, Vector3Scale(rungDir, 10000.0f));
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
        DrawLineEx(start, gapL, thick, c);
        DrawLineEx(gapR, end, thick, c);

        if (angle == 0) {
            // horizon line — ticks point down (toward ground)
            DrawLineEx(start, {
                           start.x - perp.x * tick,
                           start.y - perp.y * tick
                       }, thick, c);
            DrawLineEx(end, {
                           end.x - perp.x * tick,
                           end.y - perp.y * tick
                       }, thick, c);
        } else {
            // ticks always point toward the horizon (0°)
            const float sign = (angle > 0) ? -1.0f : 1.0f;
            DrawLineEx(start, {
                           start.x + perp.x * sign * tick,
                           start.y + perp.y * sign * tick
                       }, thick, c);
            DrawLineEx(end, {
                           end.x + perp.x * sign * tick,
                           end.y + perp.y * sign * tick
                       }, thick, c);

            DrawText(TextFormat("%d", angle),
                     static_cast<int>(start.x) - 20,
                     static_cast<int>(start.y) - 5, 10, c);
        }
    }
    EndScissorMode();
}
