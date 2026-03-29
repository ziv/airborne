#include "GameplayScreen.h"
#include "primitives/Utils.h"
#include "views/Floater.h"
#include "views/Hud.h"
#include "views/PowerGauge.h"

constexpr Vector3 l1 = {2400.0f, 200.0f, 2400};
constexpr Vector3 l2 = {2400.0f, 200.0f, 0.0f};
constexpr Vector3 l3 = {0.0f, 200.0f, 0.0f};
constexpr Vector3 l4 = {0.0f, 200.0f, 2400.0f};
constexpr Vector3 a{0.0f, 10.0f, 0.0f};

GameplayScreen::GameplayScreen(AppConfig &inputConfig) : GameScreen(inputConfig) {
    game = std::make_unique<GameData>(inputConfig);
    autopilot = std::make_unique<Autopilot>(config.maxBankAngle(), config.maxPullRatio(), config.speedRatio());

    // todo create game->SetPosition
    game->SetPosition((Vector3){0.0f, 10.0f, 0.0f});

    // todo temporary
    autopilot->AddWaypoint(Vector3Add(l1, a), 80.0f, 50.0f);
    autopilot->AddWaypoint(Vector3Add(l2, a), 60.0f, 50.0f);
    autopilot->AddWaypoint(Vector3Add(l3, a), 90.0f, 50.0f);
    autopilot->AddWaypoint(Vector3Add(l4, a), 50.0f, 50.0f);

    // shader for the cockpit image
    constexpr float thresholdValue = 0.5f;
    SetShaderValue(chromaShader, GetShaderLocation(chromaShader, "threshold"), &thresholdValue, SHADER_UNIFORM_FLOAT);

    // engine sound
    PlayMusicStream(engine);
}

GameplayScreen::~GameplayScreen() {
    UnloadTexture(cockpit);
    UnloadShader(chromaShader);
    UnloadMusicStream(engine);
}

ScreenState GameplayScreen::Update() {
    const float targetPitch = 0.8f + (game->throttle * 0.7f);
    const float targetVolume = 0.2f + (game->throttle * 0.9f);
    SetMusicPitch(engine, targetPitch);
    SetMusicVolume(engine, targetVolume);

    UpdateMusicStream(engine);
    const float deltaTime = game->Tick();

    // should be first to allow disengaged autopilot
    if (IsKeyPressed(KEY_P)) autopilot->Toggle();

    // fast return, autopilot mode, no need to get other user inputs
    if (autopilot->IsActive()) {
        // get controls from autopilot and update game state
        game->controls = autopilot->AutoSteer(*game);
        // autopilot update throttle
        game->throttle += game->controls.Throttle;
        game->throttle = Clamp(game->throttle, 0.0f, 1.2f);
        game->Update();
        return ScreenState::GAMEPLAY;
    }

    auto controls = game->ResetControls();

    // faster is better for steering, no speed -> no steer
    // auto const speed = game->Speed();
    // auto const speedEffect = speed == 0 ? 0 : Clamp(1 - (1 / game->Speed()), 0.0f, 1.0f);

    // steering

    if (IsKeyDown(KEY_UP)) controls.Pitch = -config.pitchRatio() * deltaTime;
    if (IsKeyDown(KEY_DOWN)) controls.Pitch = config.pitchRatio() * deltaTime;

    if (IsKeyDown(KEY_LEFT)) controls.Roll = -config.rollRaio() * deltaTime;
    if (IsKeyDown(KEY_RIGHT)) controls.Roll = config.rollRaio() * deltaTime;

    // // todo for debug only, user should not be allow to change YAW directly
    if (IsKeyDown(KEY_Q)) controls.Yaw = config.yawRatio() * deltaTime;
    if (IsKeyDown(KEY_E)) controls.Yaw = -config.yawRatio() * deltaTime;

    // throttling

    // set throttle directly
    constexpr float ta = 0.12375f;
    if (IsKeyDown(KEY_A)) game->throttle = 1.2f; // after burners
    if (IsKeyDown(KEY_ZERO)) game->throttle = 0.0f;
    if (IsKeyDown(KEY_ONE)) game->throttle = 0.01f;
    if (IsKeyDown(KEY_TWO)) game->throttle = 0.01f + ta;
    if (IsKeyDown(KEY_THREE)) game->throttle = 0.01f + ta * 2;
    if (IsKeyDown(KEY_FOUR)) game->throttle = 0.01f + ta * 3;
    if (IsKeyDown(KEY_FIVE)) game->throttle = 0.01f + ta * 4;
    if (IsKeyDown(KEY_SIX)) game->throttle = 0.01f + ta * 5;
    if (IsKeyDown(KEY_SEVEN)) game->throttle = 0.01f + ta * 6;
    if (IsKeyDown(KEY_EIGHT)) game->throttle = 0.01f + ta * 7;
    if (IsKeyDown(KEY_NINE)) game->throttle = 0.01f + ta * 8;

    // increase/decrease throttle
    if (IsKeyDown(KEY_MINUS)) game->throttle -= 0.005f;
    if (IsKeyDown(KEY_EQUAL)) game->throttle += 0.005f;

    if (IsKeyDown(KEY_B)) game->breaks = !game->breaks;

    // limit
    game->throttle = Clamp(game->throttle, 0.0f, 1.2f);

    game->controls = controls;
    game->Update();

    // cockpitModel.transform = MatrixMultiply(MatrixRotateY(-90 * DEG2RAD), QuaternionToMatrix(game->GetRotation()));

    return ScreenState::GAMEPLAY;
}

void GameplayScreen::Draw() {
    ClearBackground(BLUE);

    BeginMode3D(game->GetCamera());
    if (config.showGrid()) DrawGrid(100, 20.0f);

    // DrawModel(map, (Vector3){0.0f, -1200.0f, 0.0f}, 1.0f, WHITE);
    // DrawModel(cockpitModel, Vector3Subtract(game->GetPosition(), {0, 10, -1}), 10.0f, WHITE);
    DrawCube(l1, 10.0f, 10.0f, 10.0f, RED);
    DrawCube(l2, 10.0f, 10.0f, 10.0f, RED);
    DrawCube(l3, 10.0f, 10.0f, 10.0f, RED);
    DrawCube(l4, 10.0f, 10.0f, 10.0f, RED);

    EndMode3D();
    // DrawTexture(cockpit, -51, 0, WHITE);
    BeginShaderMode(chromaShader);
    // מציירים את התמונה על כל המסך
    // DrawTextureEx(cockpit, {-60, 10}, 0, 1.0f, WHITE);
    DrawTextureEx(cockpit, {-40, 0}, 0, 1.0f, WHITE);
    EndShaderMode();
    DrawPowerGauge(*game);
    DrawHud(*game);
    DrawFloater(*game);

    // DrawLegend();
    //
    // BeginMode3D(playerCamera.GetRaylibCamera());
    // // draw some "enemies"
    // DrawGrid(100, 20.0f);
    // DrawCube({500.0f, 150.0f, 500.0f}, 10.0f, 10.0f, 10.0f, RED);
    // DrawCube({400.0f, 250.0f, 200.0f}, 10.0f, 10.0f, 10.0f, RED);
    // DrawCube({200.0f, 150.0f, 400.0f}, 10.0f, 10.0f, 10.0f, RED);
    // DrawCube({100.0f, 250.0f, 100.0f}, 10.0f, 10.0f, 10.0f, RED);
    // DrawCube({-100.0f, 10.0f, -200.0f}, 10.0f, 10.0f, 10.0f, GREEN);
    // DrawCube({-100.0f, 10.0f, -100.0f}, 10.0f, 10.0f, 10.0f, GREEN);
    // DrawCube({0.0f, 10.0f, 0.0f}, 10.0f, 10.0f, 10.0f, GREEN);
    //
    // DrawModel(map, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    // EndMode3D();
    //
    // DrawTexture(cockpit, -8, 0, WHITE);
    // DrawLegend();
    // hud.Draw(playerCamera, playerInput);
    //
    // if (flightComputer.IsActive()) {
    //     DrawText("AUTOPILOT ENGAGED", GameConfig::SCREEN_WIDTH / 2 - 100, 20, 20, RED);
    // }
}

// GameplayScreen::GameplayScreen() : cockpit(LoadTexture(GameConfig::COCKPIT_OVERLAY_PATH.data())),
//                                    music(LoadMusicStream("res/jet-engine-9.mp3")) {
//     playerCamera.Place({0.0f, 15000.0f, 0.0f}, {10.0f, 500.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
//     playerInput.Speed = 0.0f;
//
//     // todo temporary auto pilot points
//     flightComputer.AddWaypoint({400.0f, 500.0f, 400}, 25.0f);
//     flightComputer.AddWaypoint({400.0f, 500.0f, 0.0f}, 25.0f);
//     flightComputer.AddWaypoint({0.0f, 500.0f, 0.0f}, 25.0f);
//     flightComputer.AddWaypoint({0.0f, 500.0f, 400.0f}, 25.0f);
//
//     // PlayMusicStream(music);
// }
