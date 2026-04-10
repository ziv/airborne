/**
 * @file MainMenuScreen.cpp
 * @brief Main menu rendering — controls help and mission selection list.
 */
#include "MainMenuScreen.h"
#include "scenario/ScenarioLoader.h"
#include "raylib.h"

MainMenuScreen::MainMenuScreen(AppConfig &config, Scenario &scenario)
    : GameScreen(config),
      scenarios(ScenarioLoader::loadAll(config.get<std::string_view>("/mainMenu/scenarios").data())),
      activeScenario(scenario),
      bg(LoadTexture(config.get<std::string_view>("/mainMenu/bgPath").data())) {
    // scenarios = ;
    scenariosCount = static_cast<int>(scenarios.size());
}

ScreenState MainMenuScreen::update() {
    if (!scenarios.empty()) {
        if (IsKeyPressed(KEY_UP)) selectedIndex = (selectedIndex - 1 + scenariosCount) % scenariosCount;
        if (IsKeyPressed(KEY_DOWN)) selectedIndex = (selectedIndex + 1) % scenariosCount;
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            activeScenario = scenarios[selectedIndex];
            return ScreenState::BRIEFING;
        }
    }
    if (IsKeyPressed(KEY_ESCAPE)) return ScreenState::SPLASH;
    if (IsKeyPressed(KEY_SLASH)) return ScreenState::HELP;
    return ScreenState::MAIN_MENU;
}

void MainMenuScreen::run() {
    DrawTexture(bg, 0, 0, WHITE);
    int y = 50;
    DrawText("Welcome caption!", 50, y, 40, DARKPURPLE);
    y += 60;

    if (!scenarios.empty()) {
        DrawText("Select Mission", 50, y, 24, WHITE);
        y += 35;
        for (int i = 0; i < scenariosCount; i++) {
            const bool selected = (i == selectedIndex);
            const auto &s = scenarios[i];

            const Color nameColor = selected ? DARKPURPLE : GOLD;

            DrawText(TextFormat("%s%s", selected ? "> " : "  ", s.name.data()), 50, y, 20, nameColor);
            DrawText(TextFormat("[%s]", s.difficulty.data()), 350, y, 16, BLACK);

            if (selected) {
                // show description for the highlighted mission
                DrawText(s.description.c_str(), 70, y + 22, 12, WHITE);
                y += 22;
            }
            y += 30;
        }
        y += 15;
        DrawText("Press [ENTER] to start selected mission", 50, y, 20, WHITE);
        y += 30;
        DrawText("Press [/] for the help page", 50, y, 16, WHITE);
    } else {
        DrawText("No scenarios found in res/config/scenarios.jsonc", 50, y, 20, RED);
    }
}
