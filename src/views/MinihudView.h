#pragma once
#include <vector>
#include "raylib.h"
#include "../core/AircraftStructs.h"
#include "../primitives/AppConfig.h"

class MinihudView {
    // configration
    int ladderX;
    int ladderY;
    int ladderWidth;
    int ladderHeight;
    float fov;
    int width;
    int height;
    float ladderOffset;

    // hud colors
    char color = 0;
    std::pmr::vector<Color> colors = {GREEN, WHITE, BLACK};

    void drawLadder(const AircraftState &state) const;
    void drawRateOfClimb(const AircraftState &state) const;
public:
    explicit MinihudView(const AppConfig &config);

    void update();

    void draw(const AircraftState &state) const;
};
