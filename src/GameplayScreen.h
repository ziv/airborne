#pragma once
#include "GameScreen.h"
#include "primitives/Autopilot.h"
#include "primitives/GameCamera.h"
#include "primitives/GameData.h"
#include "primitives/Utils.h"
#include "views/Hud.h"

class GameplayScreen : public GameScreen {
    // the object contain all relevant game details for
    // control, change and view
    GameData game{};
    // the camera is the airplane
    GameCamera playerCamera{};
    Orientation playerInput{};

    // view elements
    Hud hud{};
    Texture2D cockpit = LoadTexture(GameConfig::COCKPIT_OVERLAY_PATH.data());
    Model map = TmpLoadModel();

    // Sound
    Music music = LoadMusicStream("res/jet-engine-9.mp3");

    Autopilot flightComputer{};

    void DrawLegend() const;

public:
    using GameScreen::GameScreen;
    // GameplayScreen();

    ~GameplayScreen() override;

    ScreenState Update() override;

    void Draw() override;
};
