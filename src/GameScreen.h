/**
 * @file GameScreen.h
 * @brief Base class for the screen state machine that drives the game loop.
 *
 * Each concrete screen inherits from GameScreen and implements update() (logic)
 * and run() (rendering). The main loop swaps screens via std::make_unique on
 * ScreenState transitions.
 */
#pragma once
#include "lib/json.hpp"
#include "primitives/AppConfig.h"

using json = nlohmann::json;

/// Possible states in the screen state machine.
// todo should move out of here to reduce risk of recompile everything on adding new screen
enum class ScreenState {
    SPLASH,
    MAIN_MENU,
    BRIEFING,
    GAMEPLAY,
    DEBRIEF,
    HELP,
    EXIT
};

/**
 * @brief Abstract base class for all game screens.
 *
 * Derived screens implement update() to process input and return the next
 * ScreenState, and run() to draw the screen's visuals each frame.
 */
class GameScreen {
protected:
    AppConfig &config;

public:
    explicit GameScreen(AppConfig &inputConfig) : config(inputConfig) {
    }

    virtual ~GameScreen() = default;

    /// @brief Process input and game logic for this frame.
    /// @return The next ScreenState (return the current state to stay on this screen).
    virtual ScreenState update() = 0;

    /// @brief Draw the screen's visuals. Called between BeginDrawing/EndDrawing.
    virtual void run() = 0;
};
