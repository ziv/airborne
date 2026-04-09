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
#include <vector>
#include "GameScreen.h"
#include "core/Autopilot.h"
#include "core/GameData.h"
#include "core/SceneManager.h"
#include "views/Aircraft.h"
#include "views/CockpitView.h"
#include "views/DebugView.h"
#include "views/HudView.h"
#include "views/cockpit/MapView.h"
#include "views/cockpit/MinihudView.h"
#include "views/NavballView.h"
#include "views/cockpit/RadarView.h"

class GameplayScreen : public GameScreen {
    GameData game;       ///< Core simulation (physics, controls, entities).
    SceneManager scene;  ///< Terrain rendering, fog shader, engine audio.

    // views
    CockpitView cockpitView;
    DebugView debugView;
    MinihudView minihudView;
    NavballView navballView;
    HudView hudView;
    MapView mapView;
    // RadarView radarView;
    Aircraft aircraft;

    /// Seconds elapsed since crash — used for the crash overlay delay.
    float crashTimer = 0.0f;

public:
    /// Every mission requires a scenario (no free-flight mode).
    GameplayScreen(AppConfig &config, const Scenario &scenario);

    ScreenState update() override;
    void run() override;
};
