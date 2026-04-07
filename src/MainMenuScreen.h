#pragma once
#include "GameScreen.h"
#include "scenario/Scenario.h"
#include <vector>

class MainMenuScreen : public GameScreen {
    Scenario& activeScenario;
    std::vector<Scenario> scenarios;
    int selectedIndex = 0;

public:
    MainMenuScreen(AppConfig& config, Scenario& scenario);
    ScreenState update() override;
    void run() override;
};
