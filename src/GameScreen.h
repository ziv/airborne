#pragma once
#include "json.hpp"
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
    const AppConfig &config;

public:
    explicit GameScreen(AppConfig &inputConfig) : config(inputConfig) {
    }

    virtual ~GameScreen() = default;

    virtual ScreenState Update() = 0;

    virtual void Draw() = 0;
};
