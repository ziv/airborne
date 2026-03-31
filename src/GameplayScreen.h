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
    // std::unique_ptr<GameData> game;
    GameData game;

    // controllers
    Autopilot autopilot;

    // views
    std::vector<std::unique_ptr<View>> views;

    // cockpit
    Texture2D cockpit{};
    Shader chromaShader{};

    // background sound
    Music engine{};

    // terrain
    Model map{};

    Model futuristicCity = LoadModel("res/futuristic_city.glb");
    void handleInputs();
    void handleSounds() const;
public:
    explicit GameplayScreen(AppConfig &inputConfig);

    ~GameplayScreen() override;

    ScreenState update() override;

    void run() override;
};
