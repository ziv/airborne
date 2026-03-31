#pragma once
#include <unordered_map>
#include "raylib.h"
#include "../core/AppConfig.h"
#include "../primitives/Resource.h"

class GaugesView{
    TextureHandle sprites;
    std::unordered_map<std::string, Rectangle> map;

    // void drawSprite(const std::string &name, Vector2 position);
    // void drawPower(const GameData &game);
    // void drawBreaks(const GameData &game);
    // void drawGear(const GameData &game);

public:
    explicit GaugesView(const AppConfig &config);

    // ~GaugesView() override;

    // void update(const GameData &game) override;

    void draw();
};
