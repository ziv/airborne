#include "GameplayScreen.h"

#include "rlgl.h"
#include "primitives/Utils.h"
#include "primitives/AppConfig.h"

constexpr Vector3 l1 = {5000.0f, 1500.0f, 5000};
constexpr Vector3 l2 = {9000.0f, 1500.0f, 4000.0f};
constexpr Vector3 l3 = {10000.0f, 1500.0f, 8000.0f};
constexpr Vector3 l4 = {5500.0f, 1500.0f, 6500.0f};
constexpr Vector3 a{0.0f, 10.0f, 0.0f};


GameplayScreen::GameplayScreen(AppConfig &config)
    : GameScreen(config),
      game(config),
      scene(config),
      cockpitView(config),
      debugView(game),
      minihudView(config),
      navballView(config),
      hudView(config),
      mapView(config),
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
    // todo position should come from the mission data
    game.aircraftCamera.setPosition((Vector3){0.0f, config.get<float>("/airplane/heightAboveGround"), 0.0f});
    PlayMusicStream(engine);
}

ScreenState GameplayScreen::update() {
    // pause game
    if (IsKeyPressed(KEY_L)) game.paused = !game.paused;
    if (game.paused) return ScreenState::GAMEPLAY;

    if (IsKeyPressed(KEY_M)) {
        game.aircraftCamera.getCamera().position = (Vector3){0.0f, 3.0f, 0.0f};
    }

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
    const auto dt = GetFrameTime();
    game.update(dt);
    scene.update(game.state, dt);
    minihudView.update();
    mapView.update(game.state);

    // bg sound
    const auto throttle = game.state.controls.throttle;
    const float targetPitch = 0.8f + (throttle * 0.7f);
    const float targetVolume = 0.2f + (throttle * 0.9f);
    SetMusicPitch(engine, targetPitch);
    SetMusicVolume(engine, targetVolume);
    UpdateMusicStream(engine);

    return ScreenState::GAMEPLAY;
}

void GameplayScreen::run() {
    // @formatter:off
    ClearBackground(BLUE);

    // const auto camera = game.aircraftCamera.getCamera();
    // const auto controls = game.aircraftControls.getControls();
    // const auto speed = game.aircraftPhysics.getForces().speed;
    // const auto directions = game.aircraftTransformation.getDirections();

    // Vector3 center = Vector3Add(camera.position, Vector3Scale(directions.forward, 5.0f));
    // Vector3 x = Vector3Add(center, Vector3Scale(GamePhysics::WorldRight, 10.0f));
    // Vector3 y = Vector3Add(center, Vector3Scale(GamePhysics::WorldRight, -10.0f));
    //
    // const auto c = GetWorldToScreen(center, camera);
    // const auto cx = GetWorldToScreen(x, camera);
    // const auto cy = GetWorldToScreen(y, camera);


    BeginMode3D(game.aircraftCamera.getCamera());
        DrawGrid(100, 20.0f);
        scene.draw();
    //
    //     DrawCube(l1, 10.0f, 10.0f, 10.0f, RED);
    //     DrawCube(l2, 10.0f, 10.0f, 10.0f, RED);
    //     DrawCube(l3, 10.0f, 10.0f, 10.0f, RED);
    //     DrawCube(l4, 10.0f, 10.0f, 10.0f, RED);
    //
    //

    //     // DrawModel(futuristicCity, (Vector3){6400.0f, 0.0f, 19800.0f}, 0.005f, WHITE);
    //     // DrawModel(aircraft.model, (Vector3){10.0, 10.0, 10.0}, 1.0f, RED);
    // rlDisableDepthTest();
    // minihudView.draw(game.aircraftCamera.getCamera(), game.aircraftTransformation.getDirections(), game.aircraftTransformation.getRotation());
    // rlEnableDepthTest();

    EndMode3D();
    // minihudView.draw(game.aircraftTransformation.getRotation(),
    //     game.aircraftTransformation.getDirections(),
    //     game.aircraftCamera.getCamera());

    cockpitView.draw();
    mapView.draw();
    // hudView.draw(game.aircraftCamera.getCamera(), game.state);
    minihudView.draw(game.state);


    navballView.draw(game.state);
    // BeginMode3D(game.aircraftCamera.getCamera());
    //
    // EndMode3D();
    // DrawLineEx(c, cx, 3, BLACK);
    // DrawLineEx(c, cy, 3, WHITE);
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
