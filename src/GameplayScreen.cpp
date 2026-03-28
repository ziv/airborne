#include "GameplayScreen.h"
#include "Constants.h"
#include "primitives/Utils.h"

GameplayScreen::GameplayScreen() : cockpit(LoadTexture(GameConfig::COCKPIT_OVERLAY_PATH.data())) {
    playerCamera.Place({0.0f, 10.0f, 0.0f}, {10.0f, 10.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    playerInput.Speed = 0.0f;

    // todo temporary auto pilot points
    flightComputer.AddWaypoint({400.0f, 100.0f, 400}, 25.0f);
    flightComputer.AddWaypoint({400.0f, 100.0f, 0.0f}, 25.0f);
    flightComputer.AddWaypoint({0.0f, 200.0f, 0.0f}, 25.0f);
    flightComputer.AddWaypoint({0.0f, 100.0f, 400.0f}, 25.0f);
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
        playerInput = flightComputer.CalculateSteering(playerCamera.GetRaylibCamera().position,
                                                       playerCamera.GetForward(),
                                                       playerCamera.GetUp(),
                                                       playerCamera.GetRight(),
                                                       playerInput.Speed, deltaTime);
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
    DrawCube({400.0f, 100.0f, 400.0f}, 10.0f, 10.0f, 10.0f, RED);
    DrawCube({400.0f, 100.0f, 0.0f}, 10.0f, 10.0f, 10.0f, ORANGE);
    DrawCube({0.0f, 200.0f, 0.0f}, 10.0f, 10.0f, 10.0f, DARKBLUE);
    DrawCube({0.0f, 100.0f, 400.0f}, 10.0f, 10.0f, 10.0f, GOLD);
    DrawCube({-100.0f, 10.0f, -200.0f}, 10.0f, 10.0f, 10.0f, GREEN);
    DrawCube({-100.0f, 10.0f, -100.0f}, 10.0f, 10.0f, 10.0f, GREEN);
    DrawCube({0.0f, 10.0f, 0.0f}, 10.0f, 10.0f, 10.0f, GREEN);
    EndMode3D();

    DrawTexture(cockpit, -8, 0, WHITE);
    DrawLegend();
    hud.Draw(playerCamera, playerInput);

    if (flightComputer.IsActive()) {
        DrawText("AUTOPILOT ENGAGED", GameConfig::SCREEN_WIDTH / 2 - 100, 20, 20, RED);
    }
}
void GameplayScreen::DrawLegend() const {
    const auto p = playerCamera.GetRaylibCamera().position;
    DrawText(TextFormat("X: %0.f", p.x), 20, 40, 20, BLACK);
    DrawText(TextFormat("Z: %0.f", p.z), 20, 60, 20, BLACK);
    DrawText(TextFormat("Y: %0.f", p.y), 20, 80, 20, BLACK);
    DrawText(TextFormat("p: %f", playerInput.Pitch), 20, 100, 20, GREEN);
    DrawText(TextFormat("r: %f", playerInput.Roll), 20, 120, 20, GREEN);
    DrawText(TextFormat("y: %f", playerInput.Yaw), 20, 140, 20, GREEN);
    DrawText(TextFormat("s: %f", playerInput.Speed), 20, 160, 20, GREEN);
}
