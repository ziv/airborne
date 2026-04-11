/**
 * @file BriefingScreen.cpp
 * @brief Pre-mission briefing — objectives, intel summary, and start conditions.
 */
#include "BriefingScreen.h"
#include "raylib.h"

BriefingScreen::BriefingScreen(AppConfig &config, const Scenario &scenario)
    : GameScreen(config),
      bg(LoadTexture("res/brief.png")),
      scenario(scenario) {
}

ScreenState BriefingScreen::update() {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        return ScreenState::GAMEPLAY;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        return ScreenState::MAIN_MENU;
    }
    return ScreenState::BRIEFING;
}

void BriefingScreen::run() {
    // DrawTexture(bg, 0, 0, WHITE);
    const int screenW = GetScreenWidth();
    constexpr int margin = 50;
    int y = 40;

    // title
    DrawText("MISSION BRIEFING", margin, y, 40, GOLD);
    y += 60;

    // mission name & difficulty
    DrawText(scenario.name.c_str(), margin, y, 30, WHITE);
    y += 35;
    const auto diffText = TextFormat("Difficulty: %s", scenario.difficulty.c_str());
    DrawText(diffText, margin, y, 20, LIGHTGRAY);
    y += 40;

    // description
    DrawLine(margin, y, screenW - margin, y, DARKGRAY);
    y += 15;
    DrawText(scenario.description.c_str(), margin, y, 20, LIGHTGRAY);
    y += 50;

    // objectives
    DrawText("OBJECTIVES", margin, y, 24, GOLD);
    y += 35;
    for (const auto &obj: scenario.objectives) {
        const char *marker = obj.required ? "[REQ]" : "[OPT]";
        Color markerColor = obj.required ? RED : YELLOW;
        DrawText(marker, margin, y, 18, markerColor);
        DrawText(obj.label.c_str(), margin + 70, y, 18, WHITE);
        y += 25;
    }
    y += 20;

    // entities summary
    DrawText("INTEL", margin, y, 24, GOLD);
    y += 35;
    int enemyCount = 0;
    int friendlyCount = 0;
    for (const auto &e: scenario.entityDefinitions) {
        if (e.faction == Faction::ENEMY) enemyCount++;
        else if (e.faction == Faction::FRIENDLY) friendlyCount++;
    }
    DrawText(TextFormat("Enemy targets: %d", enemyCount), margin, y, 18, RED);
    y += 25;
    DrawText(TextFormat("Friendly assets: %d", friendlyCount), margin, y, 18, GREEN);
    y += 25;
    DrawText(TextFormat("Total entities: %d", static_cast<int>(scenario.entityDefinitions.size())), margin, y, 18, LIGHTGRAY);
    y += 40;

    // start conditions
    DrawLine(margin, y, screenW - margin, y, DARKGRAY);
    y += 15;
    DrawText(TextFormat("Start: %.0fm alt, %.0f kg fuel, heading %03.0f",
                        scenario.start.altitude, scenario.start.fuel, scenario.start.heading),
             margin, y, 18, LIGHTGRAY);
    y += 25;
    DrawText(TextFormat("Carrier launch: %s", scenario.start.carrier ? "YES" : "NO"),
             margin, y, 18, LIGHTGRAY);
    y += 50;

    // prompt
    DrawText("Press [ENTER] to launch mission", margin, y, 22, GREEN);
    DrawText("Press [ESC] to return to menu", margin, y + 30, 18, GRAY);
}
