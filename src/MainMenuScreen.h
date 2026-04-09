/**
 * @file MainMenuScreen.h
 * @brief Main menu screen — displays controls help and a mission selection list.
 *
 * Loads available scenarios from disk. The player navigates with arrow keys
 * and presses ENTER to start the selected mission (transitions to BRIEFING).
 */
#pragma once
#include "GameScreen.h"
#include "scenario/Scenario.h"
#include <vector>

#include "primitives/Resource.h"

class MainMenuScreen : public GameScreen {
    Scenario &activeScenario; ///< Reference to the shared scenario slot; written on selection.
    std::vector<Scenario> scenarios; ///< All scenarios loaded from the config file.
    int selectedIndex = 0; ///< Currently highlighted scenario index.
    TextureHandle bg;

public:
    MainMenuScreen(AppConfig &config, Scenario &scenario);

    ScreenState update() override;

    void run() override;
};
