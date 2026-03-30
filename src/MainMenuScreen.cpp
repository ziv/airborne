#include "MainMenuScreen.h"
#include "raylib.h"
#include "lib/raygui.h"

ScreenState MainMenuScreen::Update() {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        return ScreenState::GAMEPLAY;
    }
    return ScreenState::MAIN_MENU;
}

void MainMenuScreen::run() {
    DrawText("Just another screen to pass", 50, 50, 40, PURPLE);

    GuiButton((Rectangle){24, 24, 120, 30}, "#190#Show");
    // DrawText("Select mission", 100, 140, 30, GREEN);
    // DrawText("Smaller text", 100, 170, 20, GREEN);
    // DrawText("Tiny text", 100, 190, 10, GREEN);
    // DrawText("Press ENTER to start", 100, 200, 20, LIGHTGRAY);
}
