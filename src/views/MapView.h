#pragma once
#include "../primitives/View.h"


class MapView : View {
    Camera2D mapCamera = {0};
    Texture2D map = LoadTexture("res/map.png");
    float heading = 0.0f;
    float zoom = 1.0;

public:
    ~MapView() override;

    void update(const GameData &game) override;

    void draw(const GameData &game) const override;
};
