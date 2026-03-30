#pragma once
#include "lib/json.hpp"
#include "primitives/AppConfig.h"

using json = nlohmann::json;

enum class ScreenState {
    SPLASH,
    MAIN_MENU,
    GAMEPLAY,
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
