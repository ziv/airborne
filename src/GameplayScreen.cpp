/**
 * @file GameplayScreen.cpp
 * @brief In-flight gameplay — simulation update loop, 3D/2D render passes,
 *        pane switching, and crash overlay.
 */
#include "GameplayScreen.h"
#include "primitives/Utils.h"
#include "primitives/AppConfig.h"
#include "entities/Entity.h"

GameplayScreen::GameplayScreen(AppConfig &config, const Scenario &scenario)
    : GameScreen(config),
      game(config, scenario),
      scene(config),
      cockpitView(game, config),
      debugView(game),
      minihudView(config),
      navballView(config),
      mapView(config) {
}

ScreenState GameplayScreen::update() {
    // pause game
    if (IsKeyPressed(KEY_P)) game.paused = !game.paused;
    if (game.paused) return ScreenState::GAMEPLAY;

    if (game.state.crashed) {
        crashTimer += GetFrameTime();
        if (crashTimer > 50.0f || IsKeyPressed(KEY_SPACE)) {
            return ScreenState::MAIN_MENU;
        }
        return ScreenState::GAMEPLAY;
    }

    const auto dt = GetFrameTime();

    // todo put all the view in a list?!
    game.update(dt);
    scene.update(game.state, dt);
    cockpitView.update(game.state, dt);
    // minihudView.update();
    debugView.update();
    // mapView.update(game.state, dt);
    // radarView.update();

    return ScreenState::GAMEPLAY;
}

void GameplayScreen::run() {
    // @formatter:off
    const auto camera = game.aircraftCamera.getCamera();
    const auto state = game.state;
    ClearBackground(BLUE);

    BeginMode3D(camera);
        // the main scene
        scene.draw(state, camera);
        // all the entities in the scene
        game.entities.draw(state);

        // if an "F" key pressed, draw the user aircraft (see it from outside)
        // if (IsKeyDown(KEY_F1)) aircraft.draw(state);
    EndMode3D();

    // if an "F" key pressed, we do not display the cockpit
    // if (IsKeyDown(KEY_F1)) return;

    // display cockpit
    cockpitView.draw(state);

    // display cockpit items
    // mapView.draw({0.0f, 0.0f});
    // minihudView.draw(state);
    debugView.draw();

    // build radar contacts from all alive entities
    // std::vector<RadarContact> contacts;
    // game.entities.forEachAlive([&](EntityBase& e) {
    //     Color color;
    //     switch (e.faction) {
    //         case Faction::ENEMY:    color = RED;    break;
    //         case Faction::FRIENDLY: color = GREEN;  break;
    //         default:                color = GRAY;   break;
    //     }
    //     contacts.push_back({e.position, color});
    // });
    // // draw on the left pane (switch between radar, navball, hud, etc.)
    // switch (leftPane) {
    //     case 0:
    //         radarView.draw(state, contacts, (Vector2){377.0f, 666.0f});
    //         break;
    //         case 1:
    //         navballView.draw(state, (Vector2){377.0f, 666.0f});
    //     default:
    //         break;
    // }

    // --- crash overlay (drawn on top of everything) ---
    if (game.state.crashed) {
        const int sw = GetScreenWidth();
        const int sh = GetScreenHeight();
        // dim the screen
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.5f));
        // main message
        const char *msg = "CRASHED";
        const int msgW = MeasureText(msg, 40);
        DrawText(msg, sw / 2 - msgW / 2, sh / 2 - 30, 40, RED);
        // hint
        const char *hint = "Press SPACE to continue";
        const int hintW = MeasureText(hint, 20);
        DrawText(hint, sw / 2 - hintW / 2, sh / 2 + 20, 20, WHITE);
    }

    // @formatter:on
}
