#pragma once
#include "GameScreen.h"

class MainMenuScreen : public GameScreen {
public:
    using GameScreen::GameScreen;
    ScreenState Update() override;

    void Draw() override;
};
