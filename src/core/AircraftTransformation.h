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


/// @brief Holds the direction/orientation of the aircraft in 3d space
class AircraftTransformation {
    MeterPerSecond maxSpeed;          ///< Reference speed for normalising control authority.
    MeterPerSecond vleSpeed;          ///< Max gear-extended speed; above this → turbulence.
    MeterPerSecond stallSpeed;        ///< Below this speed aerodynamic effects diminish.
    Ratio bankInduceYawRatio;         ///< Adverse yaw factor from bank angle.
    Ratio liftLossPitchRatio;         ///< Nose-down pitch tendency when lift vector tilts.

    /// @brief Apply full 3-axis orientation changes while airborne.
    void flyingOrientation(AircraftState &state, float dt) const;

    /// @brief Apply restricted orientation changes while on the ground.
    void groundOrientation(AircraftState &state, float dt) const;

    /// @brief Apply a delta quaternion and re-derive basis vectors.
    static void updateOrientation(AircraftState &state, const Quaternion &qDelta) ;

public:
    explicit AircraftTransformation(const AppConfig &config);

    /// @brief Apply transformation on aircraft state (delegates to flying or ground).
    /// @param state  Mutable aircraft state.
    /// @param dt     Frame delta time (seconds).
    void update(AircraftState &state, float dt) const;
};
