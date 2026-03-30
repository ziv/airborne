#include "MainMenuScreen.h"
#include "raylib.h"
#include "lib/raygui.h"

ScreenState MainMenuScreen::update() {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        return ScreenState::GAMEPLAY;
    }
    return ScreenState::MAIN_MENU;
}

void MainMenuScreen::run() {
    DrawText("Just another screen to pass", 50, 50, 40, PURPLE);
    GuiButton((Rectangle){50, 100, 120, 30}, "#190#Exit");
}
