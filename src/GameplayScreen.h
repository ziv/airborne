#pragma once
#include "GameScreen.h"
#include "primitives/GameCamera.h"

class GameplayScreen : public GameScreen {
    GameCamera playerCamera;
    Orientation playerInput{};
public:
    GameplayScreen();
    ~GameplayScreen() override = default;

    ScreenState Update() override;
    void Draw() override;
};
