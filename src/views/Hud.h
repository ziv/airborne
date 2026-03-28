#pragma once
#include "../Constants.h"
#include "../primitives/GameCamera.h"


class Hud {
    // hud dimensions and location
    const int HudSize = 280;
    const int HudX = (GameConfig::SCREEN_WIDTH - HudSize) / 2;
    const int HudY = (GameConfig::SCREEN_HEIGHT - HudSize) / 2 - 100;

public:
    void Draw(const GameCamera &camera, const Orientation &orientation) const;
};
