/**
 * @file MapView.h
 * @brief Top-down tactical map rendered in a small cockpit panel via Camera2D.
 *
 * Displays a satellite texture with a rotatable aircraft icon. Zoom is
 * adjustable with Z/X keys.
 */
#pragma once
#include "raylib.h"
#include "../../core/AircraftStructs.h"
#include "../../primitives/AppConfig.h"
#include "../../primitives/Resource.h"

class MapView {
    TextureHandle tex;             ///< Satellite map texture.
    Camera2D mapCamera = {0};      ///< 2D camera centred on the aircraft's map position.
    float heading = 0.0f;          ///< Current compass heading for the aircraft icon.
    float zoom = 1.0;              ///< Map zoom level (adjustable with Z/X).

public:
    explicit MapView(const AppConfig &config);

    /// @brief Update map camera position and heading from aircraft state.
    void update(const AircraftState &state, float dt);

    /// @brief Render the map panel with the aircraft icon.
    void draw(const Vector2 &location);
};
