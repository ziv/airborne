#pragma once
#include "lib/json.hpp"
#include "primitives/AppConfig.h"

using json = nlohmann::json;

// todo should move out of here to reduce risk of recompile everything on adding new screen
enum class ScreenState {
    SPLASH,
    MAIN_MENU,
    BRIEFING,
    GAMEPLAY,
    DEBRIEF,
    EXIT
};

class GameScreen {
protected:
    AppConfig &config;

public:
    explicit GameScreen(AppConfig &inputConfig) : config(inputConfig) {
    }

    virtual ~GameScreen() = default;

    virtual ScreenState update() = 0;

    virtual void run() = 0;
};
