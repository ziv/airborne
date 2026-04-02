#pragma once
#include "GameScreen.h"
#include "core/Autopilot.h"
#include "core/GameData.h"
#include "views/CockpitView.h"
#include "views/DebugView.h"
#include "views/MinihudView.h"
#include "views/NavballView.h"

class GameplayScreen : public GameScreen {
    // the object contain all relevant game details for
    // control, change and view
    // std::unique_ptr<GameData> game;
    GameData game;

    // views
    CockpitView cockpitView;
    DebugView debugView;
    MinihudView minihudView;
    NavballView navballView;

    // controllers
    // Autopilot autopilot;

    // views
    // std::vector<std::unique_ptr<View>> views;

    // cockpit
    // Texture2D cockpit{};
    // Shader chromaShader{};

    // background sound
    // Music engine{};
    MusicHandle engine;

    // terrain
    ModelHandle map;

    Model futuristicCity = LoadModel("res/futuristic_city.glb");
    // void handleInputs();
    // void handleSounds() const;
public:
    explicit GameplayScreen(AppConfig &config);

    ScreenState update() override;

    void run() override;
};
