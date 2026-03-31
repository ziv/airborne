#pragma once
#include "raylib.h"
#include "../primitives/View.h"

class GaugesView : public View {
    Texture2D sprites;
    std::unordered_map<std::string, Rectangle> map;

    void drawSprite(const std::string &name, Vector2 position);
    void drawPower(const GameData &game);
    void drawBreaks(const GameData &game);
    void drawGear(const GameData &game);

public:
    explicit GaugesView(AppConfig &inputConfig);

    ~GaugesView() override;

    void update(const GameData &game) override;

    void draw(const GameData &game) override;
};
