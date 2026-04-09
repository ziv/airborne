#pragma once
#include "GameScreen.h"
#include "lib/json.hpp"
#include "primitives/AppConfig.h"
#include "primitives/Resource.h"
#include "scenario/Scenario.h"

using json = nlohmann::json;

class HelpScreen: public GameScreen {
    TextureHandle bg;
    json data;

public:
    HelpScreen(AppConfig &config, Scenario &scenario);

    ScreenState update() override;

    void run() override;
};
