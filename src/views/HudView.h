#pragma once
#include <vector>
#include "raylib.h"
#include "../core/GameData.h"

class HudView {
    char current = 0;
    std::pmr::vector<Color> colrs = {GREEN, WHITE, BLACK};

public:
    explicit HudView(AppConfig &inputConfig);

    void update(const GameData &game);

    void draw(const GameData &game);
};
