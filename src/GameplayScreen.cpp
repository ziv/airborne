#include "GameplayScreen.h"
#include "primitives/Utils.h"
#include "utils/loaders.h"
#include "primitives/AppConfig.h"

constexpr Vector3 l1 = {5000.0f, 1500.0f, 5000};
constexpr Vector3 l2 = {9000.0f, 1500.0f, 4000.0f};
constexpr Vector3 l3 = {10000.0f, 1500.0f, 8000.0f};
constexpr Vector3 l4 = {5500.0f, 1500.0f, 6500.0f};
constexpr Vector3 a{0.0f, 10.0f, 0.0f};


GameplayScreen::GameplayScreen(AppConfig &inputConfig)
    : GameScreen(inputConfig),
      game(inputConfig),
      autopilot(inputConfig.maxBankAngle(), inputConfig.maxPullRatio(), inputConfig.speedRatio()),
      cockpit(LoadTexture(inputConfig.gameCockpitTexture().data())),
      chromaShader(LoadShader(nullptr, inputConfig.gameCockpitChroma().data())),
      engine(LoadMusicStream(inputConfig.gameEngineSound().data())),
      map(UtilsLoaders::loadTerrain(
          inputConfig.gameMapTexture().data(),
          inputConfig.gameMapHeightmap().data(),
          {inputConfig.gameMapSizeX(), inputConfig.gameMapSizeY(), inputConfig.gameMapSizeZ()}
      )) {
    views.push_back(std::make_unique<MapView>(inputConfig));
    views.push_back(std::make_unique<HudView>(inputConfig));
    views.push_back(std::make_unique<GaugesView>(inputConfig));
    views.push_back(std::make_unique<DebugView>(inputConfig));


    // constructor updates
    constexpr float thresholdValue = 0.5f;
    // chromaShader = LoadShader(nullptr, config.gameCockpitChroma().data());
    SetShaderValue(chromaShader, GetShaderLocation(chromaShader, "threshold"), &thresholdValue, SHADER_UNIFORM_FLOAT);

    // todo position should come from the mission data
    game.setPosition((Vector3){6450.0f, config.heightAboveGround(), 19100.0f});

    // todo waypoints should come from the mission data
    autopilot.AddWaypoint(Vector3Add(l1, a), 200.0f, 50.0f);
    autopilot.AddWaypoint(Vector3Add(l2, a), 200.0f, 50.0f);
    autopilot.AddWaypoint(Vector3Add(l3, a), 200.0f, 50.0f);
    autopilot.AddWaypoint(Vector3Add(l4, a), 200.0f, 50.0f);

    PlayMusicStream(engine);
}

GameplayScreen::~GameplayScreen() {
    UnloadModel(map);
    UnloadTexture(cockpit);
    UnloadShader(chromaShader);
    UnloadMusicStream(engine);
}

void GameplayScreen::handleInputs() {
    // steering
    if (IsKeyDown(KEY_UP)) game.controls.Pitch = -config.pitchRatio() * game.deltaTime;
    if (IsKeyDown(KEY_DOWN)) game.controls.Pitch = config.pitchRatio() * game.deltaTime;

    if (IsKeyDown(KEY_LEFT)) game.controls.Roll = -config.rollRaio() * game.deltaTime;
    if (IsKeyDown(KEY_RIGHT)) game.controls.Roll = config.rollRaio() * game.deltaTime;

    // // todo for debug only, user should not be allow to change YAW directly
    if (IsKeyDown(KEY_Q)) game.controls.Yaw = config.yawRatio() * game.deltaTime;
    if (IsKeyDown(KEY_E)) game.controls.Yaw = -config.yawRatio() * game.deltaTime;

    // throttling
    if (IsKeyDown(KEY_ZERO)) game.throttle = 0.0f;
    if (IsKeyDown(KEY_ONE)) game.throttle = 0.1f;
    if (IsKeyDown(KEY_TWO)) game.throttle = 0.2f;
    if (IsKeyDown(KEY_THREE)) game.throttle = 0.3f;
    if (IsKeyDown(KEY_FOUR)) game.throttle = 0.4f;
    if (IsKeyDown(KEY_FIVE)) game.throttle = 0.5f;
    if (IsKeyDown(KEY_SIX)) game.throttle = 0.6f;
    if (IsKeyDown(KEY_SEVEN)) game.throttle = 0.7f;
    if (IsKeyDown(KEY_EIGHT)) game.throttle = 0.8f;
    if (IsKeyDown(KEY_NINE)) game.throttle = 0.9f;
    if (IsKeyDown(KEY_A)) game.throttle = 1.2f; // after burners

    // increase/decrease throttle
    if (IsKeyDown(KEY_MINUS)) game.throttle -= 0.05f * game.deltaTime;
    if (IsKeyDown(KEY_EQUAL)) game.throttle += 0.05f * game.deltaTime;

    // breaks
    if (IsKeyPressed(KEY_B)) game.breaks = !game.breaks;
}

void GameplayScreen::handleSounds() const {
    const float targetPitch = 0.8f + (game.throttle * 0.7f);
    const float targetVolume = 0.2f + (game.throttle * 0.9f);
    SetMusicPitch(engine, targetPitch);
    SetMusicVolume(engine, targetVolume);
    UpdateMusicStream(engine);
}

ScreenState GameplayScreen::update() {
    // pause game
    if (IsKeyPressed(KEY_L)) game.paused = !game.paused;
    if (game.paused) return ScreenState::GAMEPLAY;

    // start update
    game.tick();
    handleSounds();

    // update views
    for (const auto &view: views) {
        view->update(game);
    }

    // autopilot
    // fast return, autopilot mode, no need to get other user inputs
    if (IsKeyPressed(KEY_P)) autopilot.Toggle();
    if (autopilot.IsActive()) {
        // get controls from autopilot and update game state
        game.controls = autopilot.Steer(game.getPosition(),
                                        game.getForward(),
                                        game.getRight(),
                                        game.getUp(),
                                        game.deltaTime,
                                        game.speed);
        game.update();
        return ScreenState::GAMEPLAY;
    }

    // user inputs
    game.resetControls();
    handleInputs();
    game.update();
    return ScreenState::GAMEPLAY;
}

void GameplayScreen::run() {
    // @formatter:off
    ClearBackground(BLUE);

    BeginMode3D(game.getCamera());
        if (config.showGrid()) DrawGrid(100, 20.0f);

        DrawCube(l1, 10.0f, 10.0f, 10.0f, RED);
        DrawCube(l2, 10.0f, 10.0f, 10.0f, RED);
        DrawCube(l3, 10.0f, 10.0f, 10.0f, RED);
        DrawCube(l4, 10.0f, 10.0f, 10.0f, RED);


        DrawModel(map, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
        DrawModel(futuristicCity, (Vector3){6400.0f, 0.0f, 19800.0f}, 0.005f, WHITE);
        // DrawModel(aircraft.model, (Vector3){10.0, 10.0, 10.0}, 1.0f, RED);
    EndMode3D();

    BeginShaderMode(chromaShader);
        DrawTextureEx(cockpit, {0.0f, -270.0f}, 0, 1.0f, WHITE);
    EndShaderMode();

    for (const auto& view : views) {
        view->draw(game);
    }

    if (autopilot.IsActive()) {
        DrawText("AUTOPILOT ENGAGED", static_cast<int>(game.width) / 2 - 100, 20, 20, RED);
    }
    // @formatter:on
}
