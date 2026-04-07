#pragma once
#include "GameScreen.h"
#include "scenario/Scenario.h"

class BriefingScreen : public GameScreen {
    const Scenario& scenario;

public:
    BriefingScreen(AppConfig& config, const Scenario& scenario);

    ScreenState update() override;
    void run() override;
};
