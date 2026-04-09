/**
 * @file SplashScreen.cpp
 * @brief Splash screen — background image, music, and "press to start" logic.
 */
#include "SplashScreen.h"
#include "raylib.h"

SplashScreen::SplashScreen(AppConfig &inputConfig) : GameScreen(inputConfig),
                                                     music(LoadMusicStream(config.get<std::string_view>("/splash/musicPath").data())),
                                                     tex(LoadTexture(config.get<std::string_view>("/splash/bgPath").data())) {
    PlayMusicStream(music);
}

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
