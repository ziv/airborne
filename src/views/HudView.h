#pragma once
#include <vector>
#include "raylib.h"
#include "../core/GameData.h"

class HudView {
    int colorIndex = 0;
    std::array<Color, 3> colors = {GREEN, WHITE, BLACK};

public:
    explicit HudView(AppConfig &config);

    void update(const GameData &game);

    void draw(const Camera &camera, const AircraftState &state);
};
