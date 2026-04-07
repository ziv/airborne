#include "GameplayScreen.h"
#include "primitives/Utils.h"
#include "primitives/AppConfig.h"
#include "entities/Entity.h"

GameplayScreen::GameplayScreen(AppConfig &config, const Scenario &scenario)
    : GameScreen(config),
      game(config, scenario),
      scene(config),
      cockpitView(config),
      debugView(game),
      minihudView(config),
      navballView(config),
      hudView(config),
      mapView(config)
{
}

ScreenState GameplayScreen::update() {
    // pause game
    if (IsKeyPressed(KEY_L)) game.paused = !game.paused;
    if (game.paused) return ScreenState::GAMEPLAY;

    // --- crash state: freeze gameplay, wait for SPACE or timeout → main menu ---
    if (game.state.crashed) {
        crashTimer += GetFrameTime();
        if (crashTimer > 50.0f || IsKeyPressed(KEY_SPACE)) {
            return ScreenState::MAIN_MENU;
        }
        return ScreenState::GAMEPLAY;
    }

    const auto dt = GetFrameTime();
    game.update(dt);
    scene.update(game.state, dt);
    minihudView.update();
    debugView.update();
    mapView.update(game.state);
    radarView.update();

    return ScreenState::GAMEPLAY;
}

void GameplayScreen::run() {
    // @formatter:off
    ClearBackground(BLUE);
    BeginMode3D(game.aircraftCamera.getCamera());
        DrawGrid(100, 20.0f);
        scene.draw(game.state, game.aircraftCamera.getCamera());
        game.entities.draw(game.state);
    EndMode3D();

    cockpitView.draw(game.state);
    mapView.draw();
    minihudView.draw(game.state);
    debugView.draw();

    // build radar contacts from all alive entities
    std::vector<RadarContact> contacts;
    game.entities.forEachAlive([&](EntityBase& e) {
        Color color;
        switch (e.faction) {
            case Faction::ENEMY:    color = RED;    break;
            case Faction::FRIENDLY: color = GREEN;  break;
            default:                color = GRAY;   break;
        }
        contacts.push_back({e.position, color});
    });
    radarView.draw(game.state, contacts);
    // DrawFPS(1000, 10);

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
