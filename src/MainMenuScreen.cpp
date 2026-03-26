#include "MainMenuScreen.h"
#include "raylib.h"

ScreenState MainMenuScreen::Update() {
    if (IsKeyPressed(KEY_ENTER)) {
        return ScreenState::GAMEPLAY;
    }
    return ScreenState::MAIN_MENU;
}

void MainMenuScreen::Draw() {
    DrawText("F-15 STRIKE EAGLE", 100, 100, 40, GREEN);
    // DrawText("Press ENTER to start", 100, 200, 20, LIGHTGRAY);
}
