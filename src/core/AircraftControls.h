/**
 * @file AircraftControls.h
 * @brief Maps keyboard input to PilotControls (pitch, yaw, roll, throttle,
 *        brakes, gear).
 *
 * Sensitivity ratios are loaded from config. Throttle can be set directly
 * via number keys (0-9, A for afterburner) or adjusted incrementally (+/-).
 */
#pragma once
#include "../primitives/AppConfig.h"
#include "AircraftStructs.h"

struct AircraftControlsConfig {
    float pitchRatio = 0.7f; ///< Pitch sensitivity (degrees per second at full deflection).
    float rollRatio = 0.9f; ///< Roll sensitivity.
    float yawRatio = 0.5f; ///< Yaw (rudder) sensitivity.
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AircraftControlsConfig, pitchRatio, rollRatio, yawRatio);

class AircraftControls {
    AircraftControlsConfig conf;

public:
    explicit AircraftControls(const AppConfig &config);

    /// @brief Read keyboard input and update the pilot controls in @p state.
    /// @param state  Aircraft state whose controls field will be written.
    /// @param dt     Frame delta time (seconds) — used for rate-scaled inputs.
    void update(AircraftState &state, float dt) const;
};
