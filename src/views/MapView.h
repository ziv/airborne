#pragma once
#include "raylib.h"


class MapView {
    Camera2D mapCamera = {0};
    Texture2D map = LoadTexture("res/map.png");
    float heading = 0.0f;
    float zoom = 1.0;
    
    Shader glassShader = LoadShader(nullptr, "glass_hud.fs");
    int timeLoc = GetShaderLocation(glassShader, "time");

public:
    explicit MapView(AppConfig &inputConfig) : View(inputConfig) {
    }

    ~MapView() override;

    void update(const GameData &game) override;

    void draw(const GameData &game) override;
};
