#pragma once
#include <vector>
#include "GameScreen.h"
#include "core/Autopilot.h"
#include "core/GameData.h"
#include "core/SceneManager.h"
#include "views/Aircraft.h"
#include "views/CockpitView.h"
#include "views/DebugView.h"
#include "views/HudView.h"
#include "views/MapView.h"
#include "views/MinihudView.h"
#include "views/NavballView.h"
#include "views/RadarView.h"

class GameplayScreen : public GameScreen {
    GameData game;
    SceneManager scene;

    // views
    CockpitView cockpitView;
    DebugView debugView;
    MinihudView minihudView;
    NavballView navballView;
    HudView hudView;
    MapView mapView;
    RadarView radarView;

    // Aircraft aircraft{};

public:
    explicit GameplayScreen(AppConfig &config);
    GameplayScreen(AppConfig &config, const Scenario &scenario);

    ScreenState update() override;

    void run() override;
};
