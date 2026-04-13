/**
 * @file CockpitView.h
 * @brief Draws the 2D cockpit overlay using a chroma-keyed texture and indicator sprites.
 *
 * The cockpit image is rendered through a chroma-key shader that makes green
 * pixels transparent, allowing the 3D scene to show through the canopy.
 * Brake and gear indicator lights are drawn from a sprite sheet.
 */
#pragma once
#include "cockpit/MapView.h"
#include "cockpit/MinihudView.h"
#include "cockpit/RadarView.h"
#include "../core/AircraftStructs.h"
#include "../primitives/AppConfig.h"
#include "../primitives/Resource.h"
#include "cockpit/EngineView.h"
#include "cockpit/OffView.h"

class CockpitView {
    TextureHandle cockpitTexture; ///< Full cockpit frame image.
    ShaderHandle cockpitShader; ///< Green chroma-key transparency shader.
    TextureHandle sprites; ///< Sprite sheet for indicator lights.

    // composition of views
    OffView offView;
    MinihudView minihud;
    MapView mapView;
    RadarView radarView;
    EngineView engineView;

    int paneA = 1;
    int paneB = 0;
    int paneC = 3;

    float tempTimer = 0;
    // Vector2 paneAPosition{};
    // Vector2 paneBPosition{};
    // Vector2 paneCPosition{};

    void drawPane(const AircraftState &state, int pane, const Vector2 &position);

public:
    CockpitView(GameData &gameData, const AppConfig &config);

    void update(const AircraftState &state, float dt);

    /// @brief Draw the cockpit overlay and active indicator lights.
    void draw(const AircraftState &state);
};
