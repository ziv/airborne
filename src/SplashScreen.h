#pragma once
#include "GameScreen.h"
#include "primitives/Resource.h"

class SplashScreen : public GameScreen {
    MusicHandle music;
    TextureHandle tex;

public:
    explicit SplashScreen(AppConfig &inputConfig);

    ScreenState update() override;

    void run() override;
};
