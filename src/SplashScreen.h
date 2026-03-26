#pragma once
#include "GameScreen.h"
#include "raylib.h"

class SplashScreen : public GameScreen {
    Music music;
    Texture2D texture;

public:
    SplashScreen();

    ~SplashScreen() override;

    ScreenState Update() override;

    void Draw() override;
};
