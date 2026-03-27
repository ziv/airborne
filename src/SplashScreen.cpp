#include "SplashScreen.h"
#include "Constants.h"
#include "raylib.h"

SplashScreen::SplashScreen() : music(LoadMusicStream(GameConfig::INTRO_MUSIC_PATH.data())),
                               texture(LoadTexture(GameConfig::SPLASH_PATH.data())) {
    PlayMusicStream(music);
}

SplashScreen::~SplashScreen() {
    UnloadTexture(texture);
    UnloadMusicStream(music);
}

ScreenState SplashScreen::Update() {
    UpdateMusicStream(music);
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        return ScreenState::MAIN_MENU;
    }
    return ScreenState::SPLASH;
}

void SplashScreen::Draw() {
    DrawTexture(texture, 0, 0, WHITE);
}
