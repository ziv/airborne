#include "GameplayScreen.h"
#include "primitives/Utils.h"

GameplayScreen::GameplayScreen(AppConfig &inputConfig) : GameScreen(inputConfig) {
    game = std::make_unique<GameData>(inputConfig);
    autopilot = std::make_unique<Autopilot>(config.maxBankAngle(), config.maxPullRatio(), config.speedRatio());

    game->GetCamera().position = {0.0f, 20.0f, 0.0f};
    game->GetCamera().target = {10.0f, 10.0f, 0.0f};
    game->GetCamera().up = GamePhysics::WorldUp;
}

GameplayScreen::~GameplayScreen() {
    // UnloadMusicStream(music);
    // UnloadModel(map);
    // UnloadTexture(cockpit);
    // // UpdateMusicStream(music);
}

ScreenState GameplayScreen::Update() {
    const float deltaTime = game->Tick();

    // should be first to allow disengaged autopilot
    if (IsKeyPressed(KEY_P)) autopilot->Toggle();

    // fast return, autopilot mode, no need to get other user inputs
    if (autopilot->IsActive()) {
        // get controls from autopilot and update game state
        game->controls = autopilot->AutoSteer(*game);
        game->Update();
        return ScreenState::GAMEPLAY;
    }

    auto controls = game->ResetControls();

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
    if (IsKeyDown(KEY_A)) game->throttle = 1.2f; // after burners
    if (IsKeyDown(KEY_ZERO)) game->throttle = 0.0f;
    if (IsKeyDown(KEY_ONE)) game->throttle = 0.1f;
    if (IsKeyDown(KEY_TWO)) game->throttle = 0.2f;
    if (IsKeyDown(KEY_THREE)) game->throttle = 0.3f;
    if (IsKeyDown(KEY_FOUR)) game->throttle = 0.4f;
    if (IsKeyDown(KEY_FIVE)) game->throttle = 0.5f;
    if (IsKeyDown(KEY_SIX)) game->throttle = 0.6f;
    if (IsKeyDown(KEY_SEVEN)) game->throttle = 0.7f;
    if (IsKeyDown(KEY_EIGHT)) game->throttle = 0.8f;
    if (IsKeyDown(KEY_NINE)) game->throttle = 0.9f;

    // increase/decrease throttle
    if (IsKeyDown(KEY_MINUS)) game->throttle -= 0.001f;
    if (IsKeyDown(KEY_EQUAL)) game->throttle += 0.001f;

    if (IsKeyDown(KEY_B)) game->breaks = !game->breaks;

    // if throttle comes from the control (like autopilot)
    if (controls.Throttle != 0.0f) {
        game->throttle = controls.Throttle;
    }

    if (game->throttle > 1.2f) game->throttle = 1.2f;
    if (game->throttle < 0.0f) game->throttle = 0.0f;

    // todo check this...
    // if (const auto targetVelocity = game->throttle * config.maxSpeed(); game->velocity < targetVelocity) {
    //     game->velocity = config.acceleration() * (game->velocity != 0 ? game->velocity : 1.0f);
    // } else if (game->velocity > targetVelocity) {
    //     game->velocity = (1 / config.acceleration()) * (game->velocity != 0 ? game->velocity : 1.0f);
    // }
    // game->velocity = game->throttle * config.maxSpeed();

    game->controls = controls;
    game->Update();

    return ScreenState::GAMEPLAY;
}

void GameplayScreen::Draw() {
    ClearBackground(BLUE);
    BeginMode3D(game->GetCamera());

    // if (config.showGrid())
    DrawGrid(100, 20.0f);

    DrawCube({0.0f, 10.0f, 0.0f}, 10.0f, 10.0f, 10.0f, RED);
    DrawCube({300.0f, 10.0f, 0.0f}, 10.0f, 10.0f, 10.0f, RED);
    DrawCube({0.0f, 10.0f, 300.0f}, 10.0f, 10.0f, 10.0f, RED);
    DrawCube({300.0f, 10.0f, 300.0f}, 10.0f, 10.0f, 10.0f, RED);

    EndMode3D();
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
