#pragma once
#include "GameScreen.h"
#include "items/Aircraft.h"
#include "primitives/Autopilot.h"
#include "primitives/GameData.h"
#include "primitives/Utils.h"
#include "views/DebugView.h"
#include "views/GaugesView.h"
#include "views/HudView.h"
#include "views/MapView.h"

class GameplayScreen : public GameScreen {
    // the object contain all relevant game details for
    // control, change and view
    std::unique_ptr<GameData> game;

    // controllers
    std::unique_ptr<Autopilot> autopilot;

    // viewers
    HudView hudView{};
    DebugView debugView{};
    GaugesView gaugesView{};
    MapView mapView{};

    // cockpit
    Texture2D cockpit{};
    Shader chromaShader{};

    // background sound
    Music engine{};

    // terrain
    Model map{};

public:
    explicit GameplayScreen(AppConfig &inputConfig);

    ~GameplayScreen() override;

    ScreenState update() override;

    void run() override;
};
