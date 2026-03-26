#pragma once
#include "GameScreen.h"

class MainMenuScreen : public GameScreen {
public:
    ScreenState Update() override;

    void Draw() override;
};
