#pragma once
#include "GameScreen.h"
#include "items/Aircraft.h"
#include "primitives/Autopilot.h"
#include "primitives/GameData.h"
#include "primitives/Utils.h"
#include "utils/loaders.h"
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

    Texture2D cockpit = LoadTexture("res/cockpit-l.png");
    Shader chromaShader = LoadShader(nullptr, "shaders/chromakey.fs");
    Music engine = LoadMusicStream("res/engine.mp3");

    // todo temporary
    Aircraft aircraft{"MIG-29", "res/mig-29.glb"};

    // Model cockpitModel = LoadModel("res/mig-23-cockpit.glb");
    // Shader paintingShader = LoadShader(nullptr, "src/shaders/painting.fs");
    // RenderTexture2D target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    const Model map = UtilsLoaders::loadTerrain("res/tx.png", "res/hm.png", {32000.0f, 3200.0f, 32000.0f});

public:
    explicit GameplayScreen(AppConfig &inputConfig);

    ~GameplayScreen() override;

    ScreenState Update() override;

    void run() override;
};
