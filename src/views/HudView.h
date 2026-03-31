#pragma once
#include <vector>
#include "raylib.h"
#include "../primitives/View.h"
#include "../core/GameData.h"

class HudView : public View {
    char current = 0;
    std::pmr::vector<Color> colrs = {GREEN, WHITE, BLACK};

public:
    explicit HudView(AppConfig &inputConfig) : View(inputConfig) {
    }

    void update(const GameData &game) override;

    void draw(const GameData &game) override;
};
