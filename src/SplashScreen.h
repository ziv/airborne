#pragma once
#include "GameScreen.h"
#include "raylib.h"
#include "primitives/Resource.h"

class SplashScreen : public GameScreen {
    MusicHandle music;
    TextureHandle tex;

public:
    using GameScreen::GameScreen;
    explicit SplashScreen(AppConfig &inputConfig) : GameScreen(inputConfig),
                                                    music(LoadMusicStream(config.get<std::string_view>("/splash/musicPath").data())),
                                                    tex(LoadTexture(config.get<std::string_view>("/splash/bgPath").data())) {
        PlayMusicStream(music);
    }

    ScreenState update() override;

    void run() override;
};
