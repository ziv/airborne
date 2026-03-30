#pragma once
#include "GameScreen.h"

class MainMenuScreen : public GameScreen {
public:
    using GameScreen::GameScreen;
    ScreenState update() override;

    void run() override;
};
