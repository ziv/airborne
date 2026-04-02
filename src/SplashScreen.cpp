#include "SplashScreen.h"

ScreenState SplashScreen::update() {
    UpdateMusicStream(music);
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        return ScreenState::MAIN_MENU;
    }
    return ScreenState::SPLASH;
}

void SplashScreen::run() {
    DrawTexture(tex, 0, 0, WHITE);
    DrawText("Created by Ziv Perry", 960, 750, 20, GREEN);
}
