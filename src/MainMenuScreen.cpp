#include "MainMenuScreen.h"
#include "raylib.h"

ScreenState MainMenuScreen::update() {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        return ScreenState::GAMEPLAY;
    }
    return ScreenState::MAIN_MENU;
}

void MainMenuScreen::run() {
    DrawText("Keys", 50, 50, 40, PURPLE);
    DrawText("[G]   - Toggle gear", 50, 100, 20, GREEN);
    DrawText("[B]   - Toggle breaks", 50, 130, 20, GREEN);
    DrawText("[0]   - Engine off", 50, 160, 20, GREEN);
    DrawText("[1-9] - Engine thrust", 50, 190, 20, GREEN);
    DrawText("[A]   - After burner", 50, 220, 20, GREEN);
    DrawText("[+/-] - Increase/Decrease thrust", 50, 250, 20, GREEN);
    DrawText("[ESC] - Exit", 50, 280, 20, GREEN);
    DrawText("Press [SPACE/ENTER] to start the game", 50, 330, 20, PURPLE);
}
