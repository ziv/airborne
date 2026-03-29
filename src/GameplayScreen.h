#pragma once
#include "GameScreen.h"
#include "primitives/Autopilot.h"
#include "primitives/GameData.h"
#include "primitives/Utils.h"

class GameplayScreen : public GameScreen {
    // the object contain all relevant game details for
    // control, change and view
    std::unique_ptr<GameData> game;

    // controllers
    std::unique_ptr<Autopilot> autopilot;

    Texture2D cockpit = LoadTexture("res/cockpit-05.png");
    const Model map = TmpLoadModel();
public:
    explicit GameplayScreen(AppConfig &inputConfig);

    ~GameplayScreen() override;

    ScreenState Update() override;

    void Draw() override;
};
