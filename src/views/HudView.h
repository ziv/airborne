/**
 * @file HudView.h
 * @brief Head-up display overlay — pitch ladder, speed/altitude readouts.
 *
 * Projects 3D pitch-ladder rungs onto screen space and renders them inside
 * a clipped HUD rectangle. Color can be cycled with Alt+H.
 */
#pragma once
#include "raylib.h"
#include "../core/GameData.h"

class HudView {
    int colorIndex = 0;                          ///< Active color palette index.
    std::array<Color, 3> colors = {GREEN, WHITE, BLACK};

public:
    explicit HudView(AppConfig &config);

    /// @brief Handle the Alt+H colour-cycle input.
    void update(const GameData &game);

    /// @brief Draw the HUD (pitch ladder, speed, altitude).
    void draw(const Camera &camera, const AircraftState &state);
};
