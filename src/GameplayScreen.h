#pragma once
#include "GameScreen.h"
#include "core/Autopilot.h"
#include "core/GameData.h"
#include "core/SceneManager.h"
#include "views/CockpitView.h"
#include "views/DebugView.h"
#include "views/HudView.h"
#include "views/MapView.h"
#include "views/MinihudView.h"
#include "views/NavballView.h"

class GameplayScreen : public GameScreen {
    // the object contain all relevant game details for
    // control, change and view
    // std::unique_ptr<GameData> game;
    GameData game;
    SceneManager scene;

    // views
    CockpitView cockpitView;
    DebugView debugView;
    MinihudView minihudView;
    NavballView navballView;
    HudView hudView;
    MapView mapView;

    MusicHandle engine;
    ModelHandle map;


    // Model futuristicCity = LoadModel("res/futuristic_city.glb");
    // void handleInputs();
    // void handleSounds() const;
public:
    explicit GameplayScreen(AppConfig &config);

    ScreenState update() override;

    void run() override;
};
