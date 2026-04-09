/**
 * @file DebugView.h
 * @brief On-screen debug overlay showing aircraft position and offset coordinates.
 *
 * Toggled with Alt+D. Displays both relative (render-space) and absolute
 * (world-space) XYZ coordinates.
 */
#pragma once
#include "../core/GameData.h"

class DebugView {
    GameData &game;
    bool display = true;   ///< Visibility toggle (Alt+D).

public:
    explicit DebugView(GameData &game);

    /// @brief Handle the Alt+D toggle input.
    void update();

    /// @brief Draw the debug overlay (if visible).
    void draw() const;
};
