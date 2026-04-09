#include "HelpScreen.h"

#include "primitives/Utils.h"

HelpScreen::HelpScreen(AppConfig &config, Scenario &scenario)
    : GameScreen(config),
      bg(LoadTexture("res/brief-blue.png")),
      data(UtilsLoaders::LoadJson("res/config/help.jsonc")) {
}

ScreenState HelpScreen::update() {
    return (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) ? ScreenState::MAIN_MENU : ScreenState::HELP;
}

void HelpScreen::run() {
    // std::unordered_map<std::string, Color> colorMap = {
    //     {"RED", RED},
    //     {"BLUE", BLUE},
    //     {"GRAY", GRAY}
    // };
    int y = 40;
    DrawTexture(bg, 0, 0, WHITE);
    DrawText("CONTROLS", 50, y, 40, BLUE);
    y = 100;
    for (const auto &item: data["A"]) {
        const auto text = item["text"].get<std::string>();
        const auto control = item["control"].get<std::string>();

        DrawText(control.data(), 50, y, 20, BLACK);
        DrawText(text.data(), 300, y, 20, DARKGRAY);
        y += 36;
    }
    y = 100;
    for (const auto &item: data["B"]) {
        const auto text = item["text"].get<std::string>();
        const auto control = item["control"].get<std::string>();

        DrawText(control.data(), 600, y, 20, BLACK);
        DrawText(text.data(), 850, y, 20, DARKGRAY);
        y += 36;
    }
    y = 600;
    DrawText("Press [SPACE] or [ENTER] to go back to main menu", 50, y, 16, BLUE);
}