#pragma once
#include "raylib.h"
#include "../primitives/View.h"

class GaugesView : View {
    Texture2D off = LoadTexture("res/power-gauge-off.png");
    Texture2D green = LoadTexture("res/power-gauge-green.png");
    Texture2D yellow = LoadTexture("res/power-gauge-yellow.png");
    Texture2D red = LoadTexture("res/power-gauge-red.png");

    void drawPower(const GameData &game) const;

public:
    explicit GaugesView(AppConfig &inputConfig) : View(inputConfig) {
    }

    ~GaugesView() override;

    void update(const GameData &game) override;

    void draw(const GameData &game) const override;
};
