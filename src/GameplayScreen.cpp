/**
 * @file GameplayScreen.cpp
 * @brief In-flight gameplay — simulation update loop, 3D/2D render passes,
 *        pane switching, and crash overlay.
 */
#include "GameplayScreen.h"
#include "primitives/Utils.h"
#include "primitives/AppConfig.h"

GameplayScreen::GameplayScreen(AppConfig &config, const Scenario &scenario)
    : GameScreen(config),
      game(config, scenario),
      scene(config),
      cockpitView(game, config),
      debugView(game) {
}

ScreenState GameplayScreen::update() {
    if (IsKeyPressed(KEY_P)) game.paused = !game.paused;
    if (game.paused) return ScreenState::GAMEPLAY;

    if (game.state.crashed && IsKeyPressed(KEY_SPACE)) return ScreenState::MAIN_MENU;
    if (game.state.crashed) return ScreenState::GAMEPLAY;

    if (IsKeyPressed(KEY_T)) game.autopilot.toggle();
    game.state.autopilot = game.autopilot.isActive();

    const auto dt = GetFrameTime();

    // reset state items
    // todo make sure the entities update is really run
    // game.state.landingZone = {false, false, 0.0f};

    // update landing zone info, missiles, etc.
    EntityRegistry::get().update(game.state, dt);

    // main aircraft behavior
    game.update(dt);

    // scene (sound, clouds, etc...)
    scene.update(game.state, dt);

    // views
    cockpitView.update(game.state, dt);
    debugView.update();

    return ScreenState::GAMEPLAY;
}

void GameplayScreen::run() {
    const auto camera = game.aircraftCamera.getCamera();
    const auto state = game.state;

    ClearBackground(BLUE);
    BeginMode3D(camera);
    // @formatter:off
        // the main scene
        scene.draw(state, camera);
        // all the entities in the scene
        EntityRegistry::get().draw(state);
    // @formatter:on
    EndMode3D();

    cockpitView.draw(state);
    debugView.draw();

    // --- crash overlay (drawn on top of everything) ---
    if (game.state.crashed) crashLayout();
}

void GameplayScreen::crashLayout() {
    const int sw = GetScreenWidth();
    const int sh = GetScreenHeight();
    // dim the screen
    DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.5f));
    // main message
    const auto msg = "CRASHED";
    const int msgW = MeasureText(msg, 40);
    DrawText(msg, sw / 2 - msgW / 2, sh / 2 - 30, 40, RED);
    // hint
    const auto hint = "Press SPACE to continue";
    const int hintW = MeasureText(hint, 20);
    DrawText(hint, sw / 2 - hintW / 2, sh / 2 + 20, 20, WHITE);
}
