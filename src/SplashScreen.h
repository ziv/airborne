#pragma once
#include "Constants.h"
#include "GameScreen.h"
#include "raylib.h"

class SplashScreen : public GameScreen {
    Music music{};
    Texture2D texture{};

public:
    using GameScreen::GameScreen;

    explicit SplashScreen(AppConfig &inputConfig) : GameScreen(inputConfig) {
        music = LoadMusicStream(inputConfig.splashMusicPath().data());
        texture = LoadTexture(inputConfig.splashBgPath().data());
        PlayMusicStream(music);
    }

    ~SplashScreen() override {
        UnloadTexture(texture);
        UnloadMusicStream(music);
    }


    ScreenState Update() override {
        UpdateMusicStream(music);
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            return ScreenState::GAMEPLAY;
        }
        return ScreenState::SPLASH;
    }

    void Draw() override {
        DrawTexture(texture, 0, 0, WHITE);
    }
};
