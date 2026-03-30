#pragma once
#include "primitives/Constants.h"
#include "GameScreen.h"
#include "raylib.h"

class SplashScreen : public GameScreen {
    Music music{};
    Texture2D texture{};

public:
    using GameScreen::GameScreen;

    explicit SplashScreen(AppConfig &inputConfig) : GameScreen(inputConfig) {
        music = LoadMusicStream(config.splashMusicPath().data());
        texture = LoadTexture(config.splashBgPath().data());
        PlayMusicStream(music);
    }

    ~SplashScreen() override {
        UnloadTexture(texture);
        UnloadMusicStream(music);
    }

    ScreenState update() override {
        UpdateMusicStream(music);
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            return ScreenState::MAIN_MENU;
        }
        return ScreenState::SPLASH;
    }

    void run() override {
        DrawTexture(texture, 0, 0, WHITE);
    }
};
