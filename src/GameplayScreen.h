/**
 * @file GameplayScreen.h
 * @brief The in-flight gameplay screen — composes the simulation core, scene,
 *        and all cockpit HUD views.
 *
 * Owns GameData (simulation), SceneManager (terrain/audio), and every HUD
 * overlay view. Orchestrates the per-frame 3D/2D rendering passes and handles
 * pause, pane switching, and crash-overlay logic.
 */
#pragma once
#include "GameScreen.h"
#include "core/GameData.h"
#include "core/SceneManager.h"
#include "views/Aircraft.h"
#include "views/CockpitView.h"
#include "views/DebugView.h"

class GameplayScreen : public GameScreen {
    GameData game;       ///< Core simulation (physics, controls, entities).
    SceneManager scene;  ///< Terrain rendering, fog shader, engine audio.

    // views
    CockpitView cockpitView;
    DebugView debugView;
    Aircraft aircraft;

    /// Seconds elapsed since crash — used for the crash overlay delay.
    // float crashTimer = 0.0f;

    static void crashLayout();

public:
    /// Every mission requires a scenario (no free-flight mode).
    GameplayScreen(AppConfig &config, const Scenario &scenario);

    ScreenState update() override;
    void run() override;
};
