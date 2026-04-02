#include "GameplayScreen.h"

#include "rlgl.h"
#include "primitives/Utils.h"
#include "core/AppConfig.h"

constexpr Vector3 l1 = {5000.0f, 1500.0f, 5000};
constexpr Vector3 l2 = {9000.0f, 1500.0f, 4000.0f};
constexpr Vector3 l3 = {10000.0f, 1500.0f, 8000.0f};
constexpr Vector3 l4 = {5500.0f, 1500.0f, 6500.0f};
constexpr Vector3 a{0.0f, 10.0f, 0.0f};


GameplayScreen::GameplayScreen(AppConfig &config)
    : GameScreen(config),
      game(config),
      cockpitView(config),
      debugView(game),
      navballView(config),
      // autopilot(inputConfig.maxBankAngle, inputConfig.maxPullRatio, inputConfig.speedRatio),
      // cockpit(LoadTexture(inputConfig.gameCockpitTexture.data())),
      // chromaShader(LoadShader(nullptr, inputConfig.gameCockpitChroma.data())),
      engine(LoadMusicStream(config.get<std::string_view>("/game/engineSound").data())),
      map(UtilsLoaders::loadTerrain(
          config.get<std::string_view>("/game/mapTexture").data(),
          config.get<std::string_view>("/game/mapHeightmap").data(),
          {
              config.get<float>("/game/mapSizeX"),
              config.get<float>("/game/mapSizeY"),
              config.get<float>("/game/mapSizeZ")
          }
      )) {
    // views.push_back(std::make_unique<MapView>(inputConfig));
    // views.push_back(std::make_unique<HudView>(inputConfig));
    // views.push_back(std::make_unique<GaugesView>(inputConfig));
    // views.push_back(std::make_unique<DebugView>(inputConfig));


    // constructor updates
    // constexpr float thresholdValue = 0.5f;
    // chromaShader = LoadShader(nullptr, config.gameCockpitChroma.data());
    // SetShaderValue(chromaShader, GetShaderLocation(chromaShader, "threshold"), &thresholdValue, SHADER_UNIFORM_FLOAT);

    // todo position should come from the mission data
    game.aircraftCamera.setPosition((Vector3){0.0f, config.get<float>("/airplane/heightAboveGround"), 0.0f});

    // todo waypoints should come from the mission data
    // autopilot.AddWaypoint(Vector3Add(l1, a), 200.0f, 50.0f);
    // autopilot.AddWaypoint(Vector3Add(l2, a), 200.0f, 50.0f);
    // autopilot.AddWaypoint(Vector3Add(l3, a), 200.0f, 50.0f);
    // autopilot.AddWaypoint(Vector3Add(l4, a), 200.0f, 50.0f);

    PlayMusicStream(engine);
}

// GameplayScreen::~GameplayScreen() {
//     // UnloadTexture(cockpit);
//     // UnloadShader(chromaShader);
//     // UnloadMusicStream(engine);
// }

ScreenState GameplayScreen::update() {
    // pause game
    if (IsKeyPressed(KEY_L)) game.paused = !game.paused;
    if (game.paused) return ScreenState::GAMEPLAY;

    if (IsKeyPressed(KEY_M)) {
        game.aircraftCamera.getCamera().position = (Vector3){0.0f, 3.0f, 0.0f};
    }
    // start update
    // game.tick();
    // handleSounds();

    // update views
    // for (const auto &view: views) {
    //     view->update(game);
    // }

    // autopilot
    // fast return, autopilot mode, no need to get other user inputs
    // if (IsKeyPressed(KEY_P)) autopilot.Toggle();
    // if (autopilot.IsActive()) {
    //     // get controls from autopilot and update game state
    //     game.controls = autopilot.Steer(game.getPosition(),
    //                                     game.getForward(),
    //                                     game.getRight(),
    //                                     game.getUp(),
    //                                     game.deltaTime,
    //                                     game.speed);
    //     game.update();
    //     return ScreenState::GAMEPLAY;
    // }

    game.update();
    return ScreenState::GAMEPLAY;
}

void GameplayScreen::run() {
    // @formatter:off
    ClearBackground(BLUE);

    const auto camera = game.aircraftCamera.getCamera();
    const auto controls = game.aircraftControls.getControls();
    const auto speed = game.aircraftPhysics.getForces().speed;
    const auto directions = game.aircraftTransformation.getDirections();

    Vector3 center = Vector3Add(camera.position, Vector3Scale(directions.forward, 5.0f));
    Vector3 x = Vector3Add(center, Vector3Scale(GamePhysics::WorldRight, 10.0f));
    Vector3 y = Vector3Add(center, Vector3Scale(GamePhysics::WorldRight, -10.0f));

    const auto c = GetWorldToScreen(center, camera);
    const auto cx = GetWorldToScreen(x, camera);
    const auto cy = GetWorldToScreen(y, camera);


    BeginMode3D(game.aircraftCamera.getCamera());
        DrawGrid(100, 20.0f);

    //
    //     DrawCube(l1, 10.0f, 10.0f, 10.0f, RED);
    //     DrawCube(l2, 10.0f, 10.0f, 10.0f, RED);
    //     DrawCube(l3, 10.0f, 10.0f, 10.0f, RED);
    //     DrawCube(l4, 10.0f, 10.0f, 10.0f, RED);
    //
    //
            DrawModel(map, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    //     // DrawModel(futuristicCity, (Vector3){6400.0f, 0.0f, 19800.0f}, 0.005f, WHITE);
    //     // DrawModel(aircraft.model, (Vector3){10.0, 10.0, 10.0}, 1.0f, RED);
    // rlDisableDepthTest();
    // minihudView.draw(game.aircraftCamera.getCamera(), game.aircraftTransformation.getDirections(), game.aircraftTransformation.getRotation());
    // rlEnableDepthTest();

    EndMode3D();

    cockpitView.draw();

    BeginMode3D(game.aircraftCamera.getCamera());
        navballView.draw(camera.position, directions);
    EndMode3D();
    DrawLineEx(c, cx, 3, BLACK);
    DrawLineEx(c, cy, 3, WHITE);
    debugView.draw();
    // BeginShaderMode(chromaShader);
    //     DrawTextureEx(cockpit, {0.0f, -270.0f}, 0, 1.0f, WHITE);
    // EndShaderMode();

    // for (const auto& view : views) {
    //     view->draw(game);
    // }

    // if (autopilot.IsActive()) {
        // DrawText("AUTOPILOT ENGAGED", static_cast<int>(game.width) / 2 - 100, 20, 20, RED);
    // }
    // @formatter:on
}
