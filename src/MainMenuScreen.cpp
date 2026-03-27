#include "MainMenuScreen.h"
#include "raylib.h"

ScreenState MainMenuScreen::Update() {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        return ScreenState::GAMEPLAY;
    }
    return ScreenState::MAIN_MENU;
}

void MainMenuScreen::Draw() {
    DrawText("AIRBORNE", 100, 100, 40, GREEN);
    // DrawText("Select mission", 100, 140, 30, GREEN);
    // DrawText("Smaller text", 100, 170, 20, GREEN);
    // DrawText("Tiny text", 100, 190, 10, GREEN);
    // DrawText("Press ENTER to start", 100, 200, 20, LIGHTGRAY);
}
