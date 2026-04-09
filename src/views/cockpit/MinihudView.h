/**
 * @file MinihudView.h
 * @brief Artificial-horizon ladder, speed/altitude readouts, rate-of-climb
 *        indicator, afterburner and gear warnings.
 *
 * Renders a clipped pitch/roll ladder using rlgl matrix transforms. Displays
 * speed in knots, altitude in feet, and a vertical speed bar. Flashes a GEAR
 * warning when low and gear is retracted. Colour is cyclable with Alt+H.
 */
#pragma once
#include <vector>
#include "raylib.h"
#include "../../core/AircraftStructs.h"
#include "../../primitives/AppConfig.h"

class MinihudView {
    // layout configuration (from app.jsonc)
    int ladderX;
    int ladderY;
    int ladderWidth;
    int ladderHeight;
    float fov;
    int width;
    int height;
    float ladderOffset;

    char color = 0;       ///< Active colour palette index.
    std::pmr::vector<Color> colors = {GREEN, WHITE, BLACK};

    /// @brief Draw the pitch/roll artificial-horizon ladder.
    void drawLadder(const AircraftState &state) const;

    /// @brief Draw the vertical-speed (rate of climb) bar indicator.
    void drawRateOfClimb(const AircraftState &state) const;

public:
    explicit MinihudView(const AppConfig &config);

    /// @brief Handle the Alt+H colour-cycle input.
    void update(const AircraftState &state, float dt);

    /// @brief Draw all mini-HUD elements (ladder, speed, altitude, warnings).
    void draw(const AircraftState &state) const;
};
