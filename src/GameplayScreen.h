#pragma once
#include "GameScreen.h"
#include "primitives/Autopilot.h"
#include "primitives/GameCamera.h"
#include "views/Hud.h"

class GameplayScreen : public GameScreen {
    // the camera is the airplane
    GameCamera playerCamera;
    Orientation playerInput{};

    // view elements
    Texture2D cockpit;
    Hud hud;

    Autopilot flightComputer;
    bool autopilotEngaged = false;

    void DrawLegend() const;
public:
    GameplayScreen();

    ~GameplayScreen() override;

    ScreenState Update() override;

    void Draw() override;
};
