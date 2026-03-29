#pragma once
#include "GameScreen.h"
#include "items/Aircraft.h"
#include "primitives/Autopilot.h"
#include "primitives/GameData.h"
#include "primitives/Utils.h"
#include "utils/loaders.h"

class GameplayScreen : public GameScreen {
    // the object contain all relevant game details for
    // control, change and view
    std::unique_ptr<GameData> game;

    // controllers
    std::unique_ptr<Autopilot> autopilot;

    Texture2D cockpit = LoadTexture("res/cockpit-g1-cut.png");
    Shader chromaShader = LoadShader(nullptr, "src/shaders/chromakey.fs");
    Music engine = LoadMusicStream("res/engine.mp3");

    // todo temporary
    Aircraft aircraft{"MIG-29", "res/mig-29.glb"};

    // Model cockpitModel = LoadModel("res/mig-23-cockpit.glb");
    // Shader paintingShader = LoadShader(nullptr, "src/shaders/painting.fs");
    // RenderTexture2D target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    // const Model map = UtilsLoaders::loadTerrain("res/texture.jpg", "res/heightmap.png", {10000.0f, 5000.0f, 10000.0f});

public:
    explicit GameplayScreen(AppConfig &inputConfig);

    ~GameplayScreen() override;

    ScreenState Update() override;

    void Draw() override;
};
