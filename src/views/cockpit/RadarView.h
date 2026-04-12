/**
 * @file RadarView.h
 * @brief Top-down radar scope showing contacts relative to the player's heading.
 *
 * Draws concentric range rings and plots each RadarContact as a coloured blip.
 * Range can be cycled with the R key (10 km / 40 km / 100 km).
 */
#pragma once
#include <vector>
#include "raylib.h"
#include "../../core/AircraftStructs.h"
#include "../../core/GameData.h"
#include "../../primitives/Types.h"

/// @brief A single radar return to be plotted on the scope.
struct RadarContact {
    Vector3 worldPosition{}; ///< Absolute world position of the contact.
    Color color = RED; ///< Blip color (RED = enemy, GREEN = friendly, GRAY = neutral).
};

class RadarView {
    float displayRadius = 60.0f; ///< Screen-space radius of the radar circle (pixels).

    /// Selectable detection ranges in meters.
    static constexpr Meter RANGES[] = {3000.0f, 10000.0f, 40000.0f, 100000.0f};
    static constexpr int RANGE_COUNT = 4;
    int rangeIndex = 0; ///< Currently active range index.

    /// @brief Draw the radar scope background (range rings, nose line, range label).
    void drawScope(const Vector2 &center) const;

    static void drawAircraft(int x, int y, float heading, const Color &color);

    static void drawShip(int x, int y, const Color &color);

    static void drawSam(int x, int y, const Color &color);

    static void drawStructure(int x, int y, const Color &color);

public:
    /// @brief Handle the R key to cycle radar range.
    void update();

    /// @brief Draw the radar scope and all contacts relative to the player.
    void draw(const AircraftState &state, const Vector2 &center) const;
};
