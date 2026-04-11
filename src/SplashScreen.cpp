/**
 * @file SplashScreen.cpp
 * @brief Splash screen — background image, music, and "press to start" logic.
 */
#include "SplashScreen.h"
#include "raylib.h"

SplashScreen::SplashScreen(AppConfig &inputConfig) : GameScreen(inputConfig),
                                                     conf(config.get<SplashScreenConfig>("/splash")),
                                                     music(LoadMusicStream(conf.musicPath.c_str())),
                                                     tex(LoadTexture(conf.bgPath.c_str())) {
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
    DrawText("Created by Ziv Perry", 910, 640, 20, GOLD);
}
