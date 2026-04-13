/**
 * @file Constants.h
 * @brief World-space constants: axis definitions, gravity, and unit conversion factors.
 */
#pragma once
#include "raylib.h"

/**
 * @namespace GamePhysics
 * @brief Physical constants and unit conversions used throughout the simulation.
 */
namespace GamePhysics {
    constexpr float g = 9.81f;
    /// World-space basis vectors (right-handed: +Z forward, +Y up, -X right).
    constexpr Vector3 WorldForward = {0.0f, 0.0f, 1.0f};
    constexpr Vector3 WorldUp = {0.0f, 1.0f, 0.0f};
    constexpr Vector3 WorldRight = {-1.0f, 0.0f, 0.0f};

    /// Gravitational acceleration vector (m/s²).
    constexpr Vector3 Gravity = {0.0f, -g, 0.0f};

    /// Unit conversion factors.
    constexpr float MS_TO_KNOTS = 1.94384f;          ///< Meters/second → knots.
    constexpr float MS_TO_FPM = 196.8504f;            ///< Meters/second → feet/minute.
    constexpr float METERS_TO_FEET = 3.28084f;         ///< Meters → feet.
    constexpr float METERS_TO_NM = 0.000539957f;       ///< Meters → nautical miles.
}
