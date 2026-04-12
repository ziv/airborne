/**
 * @file AircraftTransformation.h
 * @brief Quaternion-based orientation updates — applies pilot inputs, adverse
 *        yaw, lift-loss pitch, and VLE turbulence effects.
 *
 * Operates differently in flying vs ground mode: airborne orientation uses the
 * full 3-axis rotation with aerodynamic side-effects, while ground orientation
 * restricts pitch and disables roll.
 */
#pragma once
#include "../primitives/AppConfig.h"
#include "AircraftStructs.h"
#include "../primitives/Types.h"

struct AircraftTransformationConfig {
    MeterPerSecond maxSpeed = 600.0f; ///< Reference speed for normalising control authority.
    MeterPerSecond vleSpeed = 150.0f; ///< Max gear-extended speed; above this → turbulence.
    MeterPerSecond stallSpeed = 65.0f; ///< Below this speed aerodynamic effects diminish.
    Ratio bankInduceYawRatio = 0.2f; ///< Adverse yaw factor from bank angle.
    Ratio liftLossPitchRatio = 0.1f; ///< Nose-down pitch tendency when lift vector tilts.
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AircraftTransformationConfig, maxSpeed, vleSpeed, stallSpeed, bankInduceYawRatio, liftLossPitchRatio);

/// @brief Holds the direction/orientation of the aircraft in 3d space
class AircraftTransformation {
    AircraftTransformationConfig conf;

    /// @brief Apply full 3-axis orientation changes while airborne.
    void flyingOrientation(AircraftState &state, float dt) const;

    /// @brief Apply restricted orientation changes while on the ground.
    void groundOrientation(AircraftState &state, float dt) const;

public:
    explicit AircraftTransformation(const AppConfig &config);

    /// @brief Apply transformation on aircraft state (delegates to flying or ground).
    /// @param state  Mutable aircraft state.
    /// @param dt     Frame delta time (seconds).
    void update(AircraftState &state, float dt) const;

    /// @brief Apply a delta quaternion and re-derive basis vectors.
    static void updateOrientation(AircraftState &state, const Quaternion &qDelta);
};
