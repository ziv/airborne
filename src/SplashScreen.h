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

class SplashScreen : public GameScreen {
    MusicHandle music;
    TextureHandle tex;

public:
    explicit SplashScreen(AppConfig &inputConfig);

    ScreenState update() override;
    void run() override;
};
