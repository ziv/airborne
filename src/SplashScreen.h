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
        music = LoadMusicStream(config.splashMusicPath().data());
        texture = LoadTexture(config.splashBgPath().data());
        PlayMusicStream(music);
    }

    ~SplashScreen() override {
        UnloadTexture(texture);
        UnloadMusicStream(music);
    }

    ScreenState Update() override {
        UpdateMusicStream(music);
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            return ScreenState::MAIN_MENU;
        }
        return ScreenState::SPLASH;
    }

    void run() override {
        // Rectangle src = {0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height)};
        // Rectangle dst = {0.0f, 0.0f, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
        // DrawTexturePro(texture, src, dst, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
        DrawTexture(texture, 0, 0, WHITE);
        // DrawTextureEx(texture, {0.0f, 0.0f}, 0.0f, 1.0f, WHITE);
    }
};
