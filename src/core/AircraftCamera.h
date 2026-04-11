/**
 * @file AircraftCamera.h
 * @brief First-person pilot camera derived from the aircraft's orientation.
 *
 * Applies a configurable downward tilt so the pilot looks slightly below the
 * horizon. Also supports an external chase-cam view when F1 is held.
 */
#pragma once
#include "AircraftTransformation.h"
#include "../primitives/AppConfig.h"
#include "raylib.h"

struct AircraftCameraConfig {
    float tilt = 0.45f; ///< Downward pitch offset (radians) applied to the pilot's view.
    float fov = 85.0f;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AircraftCameraConfig, tilt, fov);

class AircraftCamera {
    AircraftCameraConfig conf;
    Camera camera = {0}; ///< Raylib camera updated each frame.

public:
    explicit AircraftCamera(const AppConfig &config);

    /// @brief Update camera position and target from the current aircraft state.
    /// @param state  Current aircraft state (position + orientation).
    /// @param dt     Frame delta time (seconds).
    void update(const AircraftState &state, float dt);

    /// @brief Get a mutable reference to the internal raylib Camera.
    Camera &getCamera() { return camera; }
};
