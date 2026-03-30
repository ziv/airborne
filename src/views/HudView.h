#pragma once
#include <vector>
#include "raylib.h"
#include "../primitives/View.h"
#include "../primitives/GameData.h"

class HudView : View {
    char current = 0;
    std::pmr::vector<Color> colrs = {GREEN, WHITE, BLACK};

public:
    void update(const GameData &game) override;

    void draw(const GameData &game) const override;
};
