#include "MainMenuScreen.h"
#include "scenario/ScenarioLoader.h"
#include "raylib.h"

MainMenuScreen::MainMenuScreen(AppConfig& config, Scenario& scenario)
    : GameScreen(config), activeScenario(scenario) {
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
    return ScreenState::MAIN_MENU;
}

void MainMenuScreen::run() {
    int y = 50;
    DrawText("AIRBORNE", 50, y, 40, GOLD);
    y += 60;

    DrawText("Controls", 50, y, 24, PURPLE);
    y += 35;
    DrawText("[G]   - Toggle gear", 50, y, 20, GREEN); y += 25;
    DrawText("[B]   - Toggle brakes", 50, y, 20, GREEN); y += 25;
    DrawText("[0]   - Engine off", 50, y, 20, GREEN); y += 25;
    DrawText("[1-9] - Engine thrust", 50, y, 20, GREEN); y += 25;
    DrawText("[A]   - Afterburner", 50, y, 20, GREEN); y += 25;
    DrawText("[+/-] - Increase/Decrease thrust", 50, y, 20, GREEN); y += 25;
    DrawText("[ESC] - Exit", 50, y, 20, GREEN); y += 40;

    if (!scenarios.empty()) {
        DrawText("Select Mission", 50, y, 24, PURPLE);
        y += 35;
        for (int i = 0; i < static_cast<int>(scenarios.size()); i++) {
            const bool selected = (i == selectedIndex);
            const auto& s = scenarios[i];

            Color nameColor = selected ? GOLD : LIGHTGRAY;
            const char* prefix = selected ? "> " : "  ";
            const char* diffTag = TextFormat("[%s]", s.difficulty.c_str());

            DrawText(TextFormat("%s%s", prefix, s.name.c_str()), 50, y, 20, nameColor);
            DrawText(diffTag, 350, y, 16, DARKGRAY);

            if (selected) {
                // show description for the highlighted mission
                DrawText(s.description.c_str(), 70, y + 22, 14, Color{180, 180, 180, 255});
                y += 22;
            }
            y += 25;
        }
        y += 15;
        DrawText("Press [ENTER] to start selected mission", 50, y, 20, GREEN);
    } else {
        DrawText("No scenarios found in res/config/scenarios.jsonc", 50, y, 20, RED);
        y += 30;
        DrawText("Press [ENTER] for free flight", 50, y, 20, GREEN);
    }
}
