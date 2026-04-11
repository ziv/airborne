/**
 * @file SplashScreen.h
 * @brief Title/splash screen shown at application launch.
 *
 * Displays a background image and plays music until the player presses
 * ENTER or SPACE to advance to the main menu.
 */
#pragma once
#include "GameScreen.h"
#include "primitives/Resource.h"

struct SplashScreenConfig {
    std::string musicPath = "res/audio/splash-1.mp3";
    std::string bgPath = "res/images/splash.png";
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SplashScreenConfig, musicPath, bgPath);

class SplashScreen : public GameScreen {
    SplashScreenConfig conf;
    MusicHandle music;
    TextureHandle tex;

public:
    explicit SplashScreen(AppConfig &inputConfig);

    ScreenState update() override;

    void run() override;
};
