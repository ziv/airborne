#pragma once
#include "raylib.h"
#include "../core/AppConfig.h"
#include "../primitives/Resource.h"


class MapView {
    // configuration
    TextureHandle tex;
    // ShaderHandle glass;
    // int timeLoc = GetShaderLocation(glass, "time");

    // state
    Camera2D mapCamera = {0};
    float heading = 0.0f;
    float zoom = 1.0;

    // Shader glassShader = LoadShader(nullptr, "glass_hud.fs");

public:
    explicit MapView(const AppConfig &config);

    // ~MapView() override;

    // void update(const GameData &game) override;
    //
    // void draw(const GameData &game) override;
};
