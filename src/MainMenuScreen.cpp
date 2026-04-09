/**
 * @file MainMenuScreen.cpp
 * @brief Main menu rendering — controls help and mission selection list.
 */
#include "MainMenuScreen.h"
#include "scenario/ScenarioLoader.h"
#include "raylib.h"

MainMenuScreen::MainMenuScreen(AppConfig &config, Scenario &scenario)
    : GameScreen(config),
      activeScenario(scenario),
      bg(LoadTexture("res/brief.png")) {
    scenarios = ScenarioLoader::loadAll("res/config/scenarios.jsonc");
}

ScreenState MainMenuScreen::update() {
    if (!scenarios.empty()) {
        if (IsKeyPressed(KEY_UP)) {
            selectedIndex = (selectedIndex - 1 + static_cast<int>(scenarios.size())) % static_cast<int>(scenarios.size());
        }
        if (IsKeyPressed(KEY_DOWN)) {
            selectedIndex = (selectedIndex + 1) % static_cast<int>(scenarios.size());
        }
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (!scenarios.empty()) {
            activeScenario = scenarios[selectedIndex];
            return ScreenState::BRIEFING;
        }
        return ScreenState::GAMEPLAY;
    }
    if (IsKeyPressed(KEY_SLASH)) {
        return ScreenState::HELP;
    }
    return ScreenState::MAIN_MENU;
}

void MainMenuScreen::run() {
    DrawTexture(bg, 0, 0, WHITE);
    int y = 50;
    DrawText("AIRBORNE", 50, y, 40, MAROON);
    y += 60;

    if (!scenarios.empty()) {
        DrawText("Select Mission", 50, y, 24, DARKPURPLE);
        y += 35;
        for (int i = 0; i < static_cast<int>(scenarios.size()); i++) {
            const bool selected = (i == selectedIndex);
            const auto &s = scenarios[i];

            const Color nameColor = selected ? MAROON : DARKGRAY;

            DrawText(TextFormat("%s%s", selected ? "> " : "  ", s.name.data()), 50, y, 20, nameColor);
            DrawText(TextFormat("[%s]", s.difficulty.data()), 350, y, 16, GRAY);

            if (selected) {
                // show description for the highlighted mission
                DrawText(s.description.c_str(), 70, y + 22, 12, Color{100, 100, 100, 255});
                y += 22;
            }
            y += 30;
        }
        y += 15;
        DrawText("Press [ENTER] to start selected mission", 50, y, 20, MAROON);
        y += 30;
        DrawText("Press [/] for the help page", 50, y, 16, MAROON);
    } else {
        DrawText("No scenarios found in res/config/scenarios.jsonc", 50, y, 20, RED);
    }
    // y += 50;

    // DrawText("Controls", 50, y, 24, DARKPURPLE);
    // y += 35;
    // DrawText("[G]   - Toggle gear", 50, y, 20, GREEN);
    // y += 25;
    // DrawText("[B]   - Toggle brakes", 50, y, 20, GREEN);
    // y += 25;
    // DrawText("[0]   - Engine off", 50, y, 20, GREEN);
    // y += 25;
    // DrawText("[1-9] - Engine thrust", 50, y, 20, GREEN);
    // y += 25;
    // DrawText("[A]   - Afterburner", 50, y, 20, GREEN);
    // y += 25;
    // DrawText("[+/-] - Increase/Decrease thrust", 50, y, 20, GREEN);
    // y += 25;
    // DrawText("[ESC] - Exit", 50, y, 20, GREEN);
    // y += 40;
}
