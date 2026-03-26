#include "SplashScreen.h"
#include "raylib.h"

SplashScreen::SplashScreen() : music(LoadMusicStream("res/sound1.mp3")),
                               texture(LoadTexture("res/seorg.png")) {
    PlayMusicStream(music);
}

SplashScreen::~SplashScreen() {
    UnloadTexture(texture);
    UnloadMusicStream(music);
}

ScreenState SplashScreen::Update() {
    UpdateMusicStream(music);
    if (IsKeyPressed(KEY_SPACE)) {
        return ScreenState::MAIN_MENU;
    }
    return ScreenState::SPLASH;
}

void SplashScreen::Draw() {
    DrawTexture(texture, 0, 0, WHITE);
    // DrawText("F-15 STRIKE EAGLE", 100, 100, 40, GREEN);
    // DrawText("Press ENTER to start", 100, 200, 20, LIGHTGRAY);
}
