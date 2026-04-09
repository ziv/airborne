/**
 * @file Types.h
 * @brief Semantic type aliases and user-defined literals for physics quantities.
 *
 * All aliases resolve to `float` but give intent to parameter types. The
 * corresponding user-defined literals (e.g. `100.0_mps`, `5000.0_mtr`)
 * provide self-documenting initialisation of physical constants.
 */
#pragma once

/// Dimensionless unit (count / index).
using NoUint = float;

constexpr NoUint operator""_nut(const long double val) {
    return static_cast<NoUint>(val);
}

/// Speed in meters per second.
using MeterPerSecond = float;

constexpr MeterPerSecond operator""_mps(const long double val) {
    return static_cast<MeterPerSecond>(val);
}

/// Distance in meters.
using Meter = float;

constexpr Meter operator""_mtr(const long double val) {
    return static_cast<Meter>(val);
}

/// Force in Newtons.
using Newton = float;

constexpr Newton operator""_ntn(const long double val) {
    return static_cast<Newton>(val);
}

/// Angle in degrees.
using Degree = float;

constexpr Degree operator""_deg(const long double val) {
    return static_cast<Degree>(val);
}

/// Dimensionless ratio / coefficient.
using Ratio = float;

constexpr Ratio operator""_rto(const long double val) {
    return static_cast<Ratio>(val);
}
