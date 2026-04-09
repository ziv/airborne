/**
 * @file BriefingScreen.h
 * @brief Pre-mission briefing screen showing objectives, intel, and start conditions.
 *
 * Displayed after mission selection. The player presses ENTER to launch or
 * ESC to return to the main menu.
 */
#pragma once
#include "GameScreen.h"
#include "primitives/Resource.h"
#include "scenario/Scenario.h"

class BriefingScreen : public GameScreen {
    TextureHandle bg;
    const Scenario& scenario;

public:
    BriefingScreen(AppConfig& config, const Scenario& scenario);

    ScreenState update() override;
    void run() override;
};
