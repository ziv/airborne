/**
 * @file NavballView.h
 * @brief 3D orientation ball (navball) rendered to an off-screen texture and
 *        composited into a cockpit panel.
 *
 * A textured sphere is rotated to match the aircraft's quaternion orientation
 * and rendered via a virtual camera into a RenderTexture, then blitted to the
 * cockpit at the specified panel position.
 */
#pragma once
#include "raylib.h"
#include "../core/AircraftTransformation.h"
#include "../primitives/AppConfig.h"
#include "../primitives/Resource.h"

class NavballView {
    TextureHandle tex;         ///< Navball sphere texture (horizon bands).
    Mesh sphere;               ///< Generated sphere mesh.
    Model navball;             ///< Model wrapping the sphere + texture.

    RenderTexture2D vtex;      ///< Off-screen render target for the navball.
    Camera vcam = {0};         ///< Virtual camera aimed at the navball.

public:
    explicit NavballView(const AppConfig &config);

    /// @brief Render the navball at @p center on screen.
    void draw(const AircraftState &state, const Vector2 &center);
};
