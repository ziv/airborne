#include "GameplayScreen.h"
#include "Constants.h"

GameplayScreen::GameplayScreen() : cockpit(LoadTexture(GameConfig::COCKPIT_OVERLAY_PATH.data())) {
    playerCamera.Place({0.0f, 10.0f, -20.0f}, {0.0f, 10.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    playerInput.Speed = 5.0f;

    // todo temporary auto pilot points
    flightComputer.AddWaypoint({400.0f, 100.0f, 400.0f}, 100.0f); // נסיקה חדה ופנייה ימינה
    flightComputer.AddWaypoint({-500.0f, 200.0f, -500.0f}, 80.0f); // צלילה שמאלה והאטה
    // flightComputer.AddWaypoint({.0f, .0f, .0f}, 10.0f); // סיום
}

GameplayScreen::~GameplayScreen() {
    UnloadTexture(cockpit);
}

ScreenState GameplayScreen::Update() {
    const float deltaTime = GetFrameTime();

    // should be first to allow disengaged autopilot
    if (IsKeyPressed(KEY_P)) autopilotEngaged = !autopilotEngaged;

    // fast return, autopilot mode, no need to get other user inputs
    if (autopilotEngaged && flightComputer.IsActive()) {
        playerInput = flightComputer.CalculateSteering(playerCamera, playerInput.Speed, deltaTime);
        playerCamera.Move(playerInput);
        return ScreenState::GAMEPLAY;
    }

    playerInput.Pitch = 0.0f;
    playerInput.Yaw = 0.0f;
    playerInput.Roll = 0.0f;
    playerInput.DeltaTime = deltaTime;

    if (IsKeyDown(KEY_UP)) playerInput.Pitch = -GameConfig::PITCH_RATIO * deltaTime;
    if (IsKeyDown(KEY_DOWN)) playerInput.Pitch = GameConfig::PITCH_RATIO * deltaTime;

    if (IsKeyDown(KEY_LEFT)) playerInput.Roll = -GameConfig::ROLL_RATIO * deltaTime;
    if (IsKeyDown(KEY_RIGHT)) playerInput.Roll = GameConfig::ROLL_RATIO * deltaTime;

    // todo for debug only, user should not be allow to change YAW directly
    if (IsKeyDown(KEY_Q)) playerInput.Yaw = GameConfig::YAW_RATIO * deltaTime;
    if (IsKeyDown(KEY_E)) playerInput.Yaw = -GameConfig::YAW_RATIO * deltaTime;

    if (IsKeyDown(KEY_LEFT_SHIFT)) playerInput.Speed += GameConfig::SPEED_RATIO * deltaTime;
    if (IsKeyDown(KEY_LEFT_CONTROL)) playerInput.Speed -= GameConfig::SPEED_RATIO * deltaTime;
    if (IsKeyDown(KEY_A)) playerInput.Speed = GameConfig::MAX_SPEED;
    if (playerInput.Speed <= GameConfig::MIN_SPEED) playerInput.Speed = GameConfig::MIN_SPEED;
    if (playerInput.Speed >= GameConfig::MAX_SPEED) playerInput.Speed = GameConfig::MAX_SPEED;

    if (IsKeyDown(KEY_R)) {
        playerCamera.LevelOut(playerInput);
    } else {
        playerCamera.Move(playerInput);
    }

    return ScreenState::GAMEPLAY;
}

void GameplayScreen::Draw() {
    ClearBackground(BLUE);

    BeginMode3D(playerCamera.GetRaylibCamera());
    // draw some "enemies"
    DrawGrid(100, 20.0f);
    DrawCube({100.0f, 10.0f, -100.0f}, 10.0f, 10.0f, 10.0f, RED);
    DrawCube({300.0f, 10.0f, 300.0f}, 10.0f, 10.0f, 10.0f, MAROON);
    DrawCube({-100.0f, 10.0f, -200.0f}, 10.0f, 10.0f, 10.0f, PURPLE);
    DrawCube({-100.0f, 10.0f, -100.0f}, 10.0f, 10.0f, 10.0f, GREEN);
    DrawCube({0.0f, 10.0f, 0.0f}, 10.0f, 10.0f, 10.0f, ORANGE);
    EndMode3D();

    DrawTexture(cockpit, -8, 0, WHITE);
    DrawLegend();
    DrawHud();

    if (autopilotEngaged) {
        DrawText("AUTOPILOT ENGAGED", GameConfig::SCREEN_WIDTH / 2 - 100, 20, 20, RED);
    }
}

void GameplayScreen::DrawHud() const {
    constexpr int hudSize = 280;
    constexpr int hudX = (GameConfig::SCREEN_WIDTH - hudSize) / 2;
    constexpr int hudY = (GameConfig::SCREEN_HEIGHT - hudSize) / 2 - 100;

    BeginScissorMode(hudX, hudY, hudSize, hudSize);

    const Camera rayCam = playerCamera.GetRaylibCamera();
    const Vector3 camForward = Vector3Normalize(Vector3Subtract(rayCam.target, rayCam.position));
    const Vector3 forward = playerCamera.GetForward();
    const Vector3 up = playerCamera.GetUp();

    // --- heading projected to horizontal plane ---
    Vector3 flatForward = {forward.x, 0.0f, forward.z};
    if (Vector3Length(flatForward) < 0.001f) flatForward = {up.x, 0.0f, up.z};
    flatForward = Vector3Normalize(flatForward);

    const Vector3 flatRight = Vector3Normalize(Vector3CrossProduct(flatForward, GamePhysics::WorldUp));

    // --- screen-space sky reference (computed once, valid for all rungs) ---
    const Vector3 refPt = Vector3Add(rayCam.position, Vector3Scale(camForward, 10000.0f));
    const Vector3 skyPt = Vector3Add(refPt, Vector3Scale(GamePhysics::WorldUp, 500.0f));
    const Vector2 refScr = GetWorldToScreenEx(refPt, rayCam, GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT);
    const Vector2 skyScr = GetWorldToScreenEx(skyPt, rayCam, GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT);

    Vector2 skyRef = {skyScr.x - refScr.x, skyScr.y - refScr.y};
    if (const float skyRefLen = sqrtf(skyRef.x * skyRef.x + skyRef.y * skyRef.y); skyRefLen > 0.001f) {
        skyRef.x /= skyRefLen;
        skyRef.y /= skyRefLen;
    }

    // --- speed & altitude labels (relative to HUD rect) ---
    DrawText(TextFormat("%0.f", playerInput.Speed), hudX + 10, hudY + hudSize / 2, 15, GREEN);
    DrawText(TextFormat("%0.f", rayCam.position.y), hudX + hudSize - 30, hudY + hudSize / 2, 15, GREEN);

    // --- pitch ladder ---
    for (int angle = -80; angle <= 80; angle += 20) {
        const float pitchRad = static_cast<float>(angle) * PI / 180.0f;
        const Vector3 rungDir = Vector3RotateByAxisAngle(flatForward, flatRight, pitchRad);

        // cull if behind camera
        if (Vector3DotProduct(rungDir, camForward) < 0.1f) continue;

        // project rung center AND one offset point to get screen-space direction
        const Vector3 center3D = Vector3Add(rayCam.position, Vector3Scale(rungDir, 10000.0f));
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
        constexpr float gapHalf = 30.0f;
        const float thick = (angle == 0) ? 2.0f : 1.0f;
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

void GameplayScreen::DrawLegend() const {
    const auto p = playerCamera.GetRaylibCamera().position;
    DrawText(TextFormat("X: %0.f", p.x), 20, 40, 20, BLACK);
    DrawText(TextFormat("Z: %0.f", p.z), 20, 60, 20, BLACK);
    DrawText(TextFormat("Y: %0.f", p.y), 20, 80, 20, BLACK);
    // DrawText(TextFormat("p: %0.f", playerInput.Pitch), 20, 100, 20, GREEN);
    // DrawText(TextFormat("r: %0.f", playerInput.Roll), 20, 120, 20, GREEN);
    // DrawText(TextFormat("y: %0.f", playerInput.Yaw), 20, 140, 20, GREEN);
    // DrawText(TextFormat("s: %0.f", playerInput.Speed), 20, 160, 20, GREEN);
}
