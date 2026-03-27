#pragma once
#include "GameScreen.h"
#include "primitives/Autopilot.h"
#include "primitives/GameCamera.h"

class GameplayScreen : public GameScreen {
    // the camera is the airplane
    GameCamera playerCamera;
    Orientation playerInput{};
    Texture2D cockpit;

    Autopilot flightComputer;
    bool autopilotEngaged = false;

    void DrawHud() const;
    void DrawLegend() const;
public:
    GameplayScreen();

    ~GameplayScreen() override;

    ScreenState Update() override;

    void Draw() override;
};
