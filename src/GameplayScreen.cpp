#include "GameplayScreen.h"
#include "Constants.h"

GameplayScreen::GameplayScreen() : cockpit(LoadTexture(GameConfig::COCKPIT_OVERLAY_PATH.data())) {
    playerCamera.place({0.0f, 10.0f, -20.0f}, {0.0f, 10.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    playerInput.Speed = 5.0f;
}

GameplayScreen::~GameplayScreen() {
    UnloadTexture(cockpit);
}

ScreenState GameplayScreen::Update() {
    playerInput.Pitch = 0.0f;
    playerInput.Yaw = 0.0f;
    playerInput.Roll = 0.0f;

    const float deltaTime = GetFrameTime();
    playerInput.DeltaTime = deltaTime;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) playerInput.Pitch = -1.0f * deltaTime;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) playerInput.Pitch = 1.0f * deltaTime;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) playerInput.Roll = -1.0f * deltaTime;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) playerInput.Roll = 1.0f * deltaTime;

    // todo for debug only, not suppose to be part of the game
    if (IsKeyDown(KEY_Q)) playerInput.Yaw = 0.5f * deltaTime;
    if (IsKeyDown(KEY_E)) playerInput.Yaw = -0.5f * deltaTime;

    if (IsKeyDown(KEY_LEFT_SHIFT)) playerInput.Speed += 20.0f * deltaTime;
    if (IsKeyDown(KEY_LEFT_CONTROL)) playerInput.Speed -= 20.0f * deltaTime;
    if (playerInput.Speed <= 0.0f) playerInput.Speed = 0.0f;
    if (playerInput.Speed > 2000.0f) playerInput.Speed = 2000.0f;

    if (IsKeyDown(KEY_R)) {
        playerCamera.levelOut(1.0f, playerInput);
    } else {
        playerCamera.move(playerInput);
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
    DrawCube({-100.0f, 10.0f, -100.0f}, 10.0f, 10.0f, 10.0f, PURPLE);
    DrawCube({-100.0f, 10.0f, -100.0f}, 10.0f, 10.0f, 10.0f, GREEN);
    DrawCube({0.0f, 10.0f, 0.0f}, 10.0f, 10.0f, 10.0f, ORANGE);
    EndMode3D();

    DrawTexture(cockpit, -8, 0, WHITE);
    DrawLegend();
    DrawHud();
}

void GameplayScreen::DrawHud() const {
    constexpr int hudSize = 280;
    constexpr int hudX = (GameConfig::SCREEN_WIDTH - hudSize) / 2;
    constexpr int hudY = (GameConfig::SCREEN_HEIGHT - hudSize) / 2 - 100;

    BeginScissorMode(hudX, hudY, hudSize, hudSize);

    // Speed label
    DrawText(TextFormat("%0.f", playerInput.Speed), 510, 260, 15, GREEN);

    // Height label todo this is absolute, replace with relative
    DrawText(TextFormat("%0.f", playerCamera.GetRaylibCamera().position.y), 760, 260, 15, GREEN);

    // details required to create the pitch ladder
    const Vector3 forward = playerCamera.GetForward();
    const Vector3 up = playerCamera.GetUp();
    const Vector3 right = playerCamera.GetRight();

    // this vector is always normal to the horizon
    Vector3 horizonDir3D = Vector3CrossProduct(forward, GamePhysics::WorldUp);

    // protect the value in case of edges
    horizonDir3D = Vector3Length(horizonDir3D) < 0.001f ? right : Vector3Normalize(horizonDir3D);

    // get the slope
    const float dx = Vector3DotProduct(horizonDir3D, right);
    const float dy = -Vector3DotProduct(horizonDir3D, up);

    // the "sky" direction
    const float skyDirX = dy;
    const float skyDirY = -dx;

    // calculate the pitch in rads
    const float pitch = asinf(forward.y);

    // some ratios
    constexpr float fovRadians = 85.0f * PI / 180.0f;
    constexpr float pixelsPerRadian = GameConfig::SCREEN_HEIGHT / fovRadians;

    // take an imaginary point in front of the plane nose (long distance)
    const Vector3 nose3D = Vector3Add(playerCamera.GetRaylibCamera().position,
                                      Vector3Scale(forward, 10000.0f));

    // find this point as vector of screen location (this is the center of the HUD)
    const auto [centerX, centerY] = GetWorldToScreenEx(nose3D,
                                                       playerCamera.GetRaylibCamera(),
                                                       GameConfig::SCREEN_WIDTH,
                                                       GameConfig::SCREEN_HEIGHT);

    // creating the pitch ladder (-80 to 80 deg)
    for (int currentAngle = -80; currentAngle <= 80; currentAngle += 20) {
        // deg to rad
        const float targetPitch = static_cast<float>(currentAngle) * PI / 180.0f;

        // how many rads from the center
        const float deltaPitch = targetPitch - pitch;

        // rads to pixels
        const float pixelDist = deltaPitch * pixelsPerRadian;

        // middle point
        const float rungCenterX = centerX + skyDirX * pixelDist;
        const float rungCenterY = centerY + skyDirY * pixelDist;

        // line 0 is longer
        const float rungLength = (currentAngle == 0) ? 110.0f : 90.0f;

        // the ends of line
        const Vector2 startPos = {rungCenterX - dx * rungLength, rungCenterY - dy * rungLength};
        const Vector2 endPos = {rungCenterX + dx * rungLength, rungCenterY + dy * rungLength};

        // a space in the middle
        const Vector2 midLeft = {rungCenterX - dx * 30.0f, rungCenterY - dy * 30.0f};
        const Vector2 midRight = {rungCenterX + dx * 30.0f, rungCenterY + dy * 30.0f};

        // line 0 is thicker
        const float thickness = (currentAngle == 0) ? 2.0f : 1.0f;

        DrawLineEx(startPos, midLeft, thickness, GREEN);
        DrawLineEx(midRight, endPos, thickness, GREEN);

        // lines wings
        if (currentAngle == 0) {
            DrawLineEx(startPos, {startPos.x + dy * 15.0f, startPos.y - dx * 15.0f}, thickness, GREEN);
            DrawLineEx(endPos, {endPos.x + dy * 15.0f, endPos.y - dx * 15.0f}, thickness, GREEN);
        } else {
            // the direction of the wings is always to the zero
            const float tickDirX = (currentAngle > 0) ? -skyDirX : skyDirX;
            const float tickDirY = (currentAngle > 0) ? -skyDirY : skyDirY;

            DrawLineEx(startPos, {startPos.x + tickDirX * 15.0f, startPos.y + tickDirY * 15.0f}, thickness, GREEN);
            DrawLineEx(endPos, {endPos.x + tickDirX * 15.0f, endPos.y + tickDirY * 15.0f}, thickness, GREEN);

            DrawText(TextFormat("%d", currentAngle), static_cast<int>(startPos.x) - 35,
                     static_cast<int>(startPos.y) - 10, 10,GREEN);
        }
    }
    EndScissorMode();
}

void GameplayScreen::DrawLegend() const {
    DrawText(TextFormat("X: %0.f", playerCamera.GetRaylibCamera().position.x), 20, 40, 20, BLACK);
    DrawText(TextFormat("Z: %0.f", playerCamera.GetRaylibCamera().position.z), 20, 60, 20, BLACK);
    DrawText(TextFormat("Y: %0.f", playerCamera.GetRaylibCamera().position.y), 20, 80, 20, BLACK);
}
