/**
 * @file AircraftPhysics.h
 * @brief Force-based flight dynamics — thrust, drag, lift, gravity, and ground
 *        handling.
 *
 * Applies Newtonian mechanics each frame: forces → acceleration → velocity →
 * position. Handles separate drag models for airborne vs ground mode, fuel
 * consumption, stall behaviour, weathervaning, and the large-world coordinate
 * shift to maintain floating-point precision.
 */
#pragma once
#include "../primitives/AppConfig.h"
#include "../primitives/Types.h"
#include "AircraftStructs.h"

class AircraftPhysics {
    // -- configuration (loaded from app.jsonc at init time) --
    Newton weight;                   ///< Aircraft empty weight (Newtons).
    Newton engineThrust;             ///< Maximum engine thrust at full military power (N).
    MeterPerSecond maxSpeed;         ///< Hard speed cap (m/s) — safety limit.
    MeterPerSecond stallSpeed;       ///< Speed below which lift drops sharply.
    MeterPerSecond groundBrakesSpeed;///< Speed threshold for ground braking damping.
    Ratio dragCoefficient;           ///< Base aerodynamic drag coefficient.
    Ratio liftCoefficient;           ///< Base lift coefficient (proportional to v²).
    Ratio flyingBrakesDragRatio;     ///< Drag multiplier when air brakes are deployed.
    Ratio flyingGearDragRatio;       ///< Extra drag when gear is deployed in flight.
    Ratio groundBrakesDragRatio;     ///< Drag multiplier for wheel brakes on the ground.
    Ratio stallLiftRatio;            ///< Lift reduction factor below stall speed.

public:
    explicit AircraftPhysics(const AppConfig &config);

    /// @brief Compute forces, integrate velocity/position, and apply ground clamping.
    /// @param state  Mutable aircraft state (forces, velocity, position updated in place).
    /// @param dt     Frame delta time (seconds).
    void update(AircraftState &state, float dt) const;
};
