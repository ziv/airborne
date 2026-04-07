#include "GameplayScreen.h"
#include "primitives/Utils.h"
#include "primitives/AppConfig.h"

GameplayScreen::GameplayScreen(AppConfig &config)
    : GameScreen(config),
      game(config),
      scene(config),
      cockpitView(config),
      debugView(game),
      minihudView(config),
      navballView(config),
      hudView(config),
      mapView(config)
{
    // todo position should come from the mission data
    // game.state.position = (Vector3){2000.0f, config.get<float>("/airplane/heightAboveGround"), 3000.0f};
    game.state.position = (Vector3){2000.0f, 100.0f, 1500.0f};
}

ScreenState GameplayScreen::update() {
    // pause game
    if (IsKeyPressed(KEY_L)) game.paused = !game.paused;
    if (game.paused) return ScreenState::GAMEPLAY;

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
    debugView.update();
    mapView.update(game.state);
    aircraft.update(game.state, dt);
    radarView.update();

    return ScreenState::GAMEPLAY;
}

void GameplayScreen::run() {
    // @formatter:off
    ClearBackground(BLUE);
    BeginMode3D(game.aircraftCamera.getCamera());
        DrawGrid(100, 20.0f);
        scene.draw(game.state, game.aircraftCamera.getCamera());
        aircraft.draw();
    EndMode3D();

    cockpitView.draw(game.state);
    mapView.draw();
    // hudView.draw(game.aircraftCamera.getCamera(), game.state);
    minihudView.draw(game.state);
    // navballView.draw(game.state);
    debugView.draw();
    const std::vector<RadarContact> contacts = {{aircraft.position, RED}};
    radarView.draw(game.state, contacts);
    DrawFPS(1000, 10);
    // @formatter:on
}
