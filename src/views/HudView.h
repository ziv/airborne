#pragma once
#include <vector>
#include "raylib.h"
#include "../core/GameData.h"

class HudView {
    char color = 0;
    std::pmr::vector<Color> colors = {GREEN, WHITE, BLACK};

public:
    explicit HudView(AppConfig &config);

    void update(const GameData &game);

    void draw(const Camera &camera, const AircraftState &state);
};
