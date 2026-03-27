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
    // hud position
    constexpr int hudSize = 280;
    constexpr int hudX = (GameConfig::SCREEN_WIDTH - hudSize) / 2;
    constexpr int hudY = (GameConfig::SCREEN_HEIGHT - hudSize) / 2 - 100;

    BeginScissorMode(hudX, hudY, hudSize, hudSize);

    // Speed label
    DrawText(TextFormat("%0.f", playerInput.Speed), 510, 260, 15, GREEN);

    // Height label todo this is absolute, replace with relative after we will add the terrain
    DrawText(TextFormat("%0.f", playerCamera.GetRaylibCamera().position.y), 760, 260, 15, GREEN);

    // todo rethink this calculation (from this point to the end of this function), it should be simpler and right now it not working well

    const Camera rayCam = playerCamera.GetRaylibCamera();
    const Vector3 camForward = Vector3Normalize(Vector3Subtract(rayCam.target, rayCam.position));

    const Vector3 forward = playerCamera.GetForward();
    const Vector3 up = playerCamera.GetUp();
    const Vector3 right = playerCamera.GetRight();
    //
    // // point somewhere in front of us
    // const Vector3 nose3D = Vector3Add(rayCam.position, Vector3Scale(forward, 10000.0f));
    // // the real center (nose of the plan)
    // const auto [x, y] = GetWorldToScreenEx(nose3D, rayCam, GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT);

    // todo temporary cross
    // DrawLineEx({x - 15, y}, {x + 15, y}, 2.0f, GREEN);
    // DrawLineEx({x, y - 15}, {x, y + 15}, 2.0f, GREEN);

    // forward with the horizon
    Vector3 flatForward = {forward.x, 0.0f, forward.z};
    if (Vector3Length(flatForward) < 0.001f) flatForward = {up.x, 0.0f, up.z};
    flatForward = Vector3Normalize(flatForward);

    // right with the horizon
    const Vector3 flatRight = Vector3Normalize(Vector3CrossProduct(flatForward, GamePhysics::WorldUp));

    Vector3 horizonDir3D = Vector3CrossProduct(forward, GamePhysics::WorldUp);
    if (Vector3Length(horizonDir3D) < 0.001f) horizonDir3D = right;
    else horizonDir3D = Vector3Normalize(horizonDir3D);

    // slop
    const float dx = Vector3DotProduct(horizonDir3D, right);
    const float dy = -Vector3DotProduct(horizonDir3D, up);

    const float skyDirX = dy;
    const float skyDirY = -dx;

    for (int currentAngle = -80; currentAngle <= 80; currentAngle += 20) {
        const float targetPitch = static_cast<float>(currentAngle) * PI / 180.0f;

        // direction in the space
        const Vector3 rungDir = Vector3RotateByAxisAngle(flatForward, flatRight, targetPitch);

        // is it in front of us?
        if (Vector3DotProduct(rungDir, camForward) < 0.1f) continue;

        // convert angles to pixels
        const Vector3 rungPos3D = Vector3Add(rayCam.position, Vector3Scale(rungDir, 10000.0f));
        const Vector2 rungCenter = GetWorldToScreenEx(rungPos3D, rayCam, GameConfig::SCREEN_WIDTH,
                                                      GameConfig::SCREEN_HEIGHT);

        const float rungLength = (currentAngle == 0) ? 110.0f : 90.0f;
        const Vector2 startPos = {rungCenter.x - dx * rungLength, rungCenter.y - dy * rungLength};
        const Vector2 endPos = {rungCenter.x + dx * rungLength, rungCenter.y + dy * rungLength};

        const Vector2 midLeft = {rungCenter.x - dx * 30.0f, rungCenter.y - dy * 30.0f};
        const Vector2 midRight = {rungCenter.x + dx * 30.0f, rungCenter.y + dy * 30.0f};

        const float thickness = (currentAngle == 0) ? 2.0f : 1.0f;

        DrawLineEx(startPos, midLeft, thickness, GREEN);
        DrawLineEx(midRight, endPos, thickness, GREEN);

        if (currentAngle == 0) {
            DrawLineEx(startPos, {startPos.x + dy * 15.0f, startPos.y - dx * 15.0f}, thickness, GREEN);
            DrawLineEx(endPos, {endPos.x + dy * 15.0f, endPos.y - dx * 15.0f}, thickness, GREEN);
        } else {
            const float tickDirX = (currentAngle > 0) ? -skyDirX : skyDirX;
            const float tickDirY = (currentAngle > 0) ? -skyDirY : skyDirY;

            DrawLineEx(startPos, {startPos.x + tickDirX * 15.0f, startPos.y + tickDirY * 15.0f}, thickness, GREEN);
            DrawLineEx(endPos, {endPos.x + tickDirX * 15.0f, endPos.y + tickDirY * 15.0f}, thickness, GREEN);
            // DrawText(TextFormat("%d", currentAngle), static_cast<int>(startPos.x) - 35, static_cast<int>(startPos.y) - 10, 20, GREEN);
            DrawText(TextFormat("%d", currentAngle), static_cast<int>(startPos.x) - 20,
                     static_cast<int>(startPos.y) - 5, 10,GREEN);
        }
    }
    EndScissorMode();
}

void GameplayScreen::DrawLegend() const {
    DrawText(TextFormat("X: %0.f", playerCamera.GetRaylibCamera().position.x), 20, 40, 20, BLACK);
    DrawText(TextFormat("Z: %0.f", playerCamera.GetRaylibCamera().position.z), 20, 60, 20, BLACK);
    DrawText(TextFormat("Y: %0.f", playerCamera.GetRaylibCamera().position.y), 20, 80, 20, BLACK);
}
