#include "GameplayScreen.h"

#include "Constants.h"

GameplayScreen::GameplayScreen() {
    playerCamera.place({0.0f, 10.0f, -20.0f}, {0.0f, 10.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    playerInput.Speed = 5.0f;
}

ScreenState GameplayScreen::Update() {
    playerInput.Pitch = 0.0f;
    playerInput.Yaw = 0.0f;
    playerInput.Roll = 0.0f;

    const float deltaTime = GetFrameTime();
    const float turnSpeed = 1.5f * deltaTime;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) playerInput.Pitch = -turnSpeed;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) playerInput.Pitch = turnSpeed;

    // גלגול (Roll)
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) playerInput.Roll = -turnSpeed;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) playerInput.Roll = turnSpeed;

    // סבסוב (Yaw) - ההגה כיוון בזנב המטוס
    if (IsKeyDown(KEY_Q)) playerInput.Yaw = turnSpeed;
    if (IsKeyDown(KEY_E)) playerInput.Yaw = -turnSpeed;

    // שליטה במצערת (מהירות)
    if (IsKeyDown(KEY_LEFT_SHIFT)) playerInput.Speed += 10.0f * deltaTime; //  * deltaTime; // האצה
    if (IsKeyDown(KEY_LEFT_CONTROL)) playerInput.Speed -= 10.0f * deltaTime; //  * deltaTime; // האטה

    playerCamera.move(playerInput);

    return ScreenState::GAMEPLAY;
}

void GameplayScreen::Draw() {
    ClearBackground(SKYBLUE);
    BeginMode3D(playerCamera.GetRaylibCamera());
    DrawGrid(100, 20.0f);
    DrawCube({0.0f, 10.0f, 100.0f}, 10.0f, 10.0f, 10.0f, RED);
    DrawCubeWires({0.0f, 10.0f, 100.0f}, 10.0f, 10.0f, 10.0f, MAROON);
    EndMode3D();

    DrawText(TextFormat("SPEED: %0.f", playerInput.Speed), 20, 20, 20, GREEN);
    DrawText("Use WASD/Arrows to fly. Q/E to Yaw. Shift/Ctrl for Speed.", 20, 50, 20, LIGHTGRAY);

    // כוונת צלב פשוטה באמצע המסך הוירטואלי שלנו
    constexpr int centerX = GameConfig::SCREEN_WIDTH / 2;
    constexpr int centerY = GameConfig::SCREEN_HEIGHT / 2;
    DrawLine(centerX - 15, centerY, centerX + 15, centerY, GREEN);
    DrawLine(centerX, centerY - 15, centerX, centerY + 15, GREEN);
}
