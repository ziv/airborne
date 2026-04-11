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

struct AircraftPhysicsConfig {
    Newton weight = 120000.0f; ///< Aircraft empty weight (Newtons).
    Newton engineThrust = 130000.0f; ///< Maximum engine thrust at full military power (N).
    MeterPerSecond maxSpeed = 600.0f; ///< Hard speed cap (m/s) — safety limit.
    MeterPerSecond stallSpeed = 65.0f; ///< Speed below which lift drops sharply.
    MeterPerSecond groundBrakesSpeed = 10.0f; ///< Speed threshold for ground braking damping.
    Ratio dragCoefficient = 0.36f; ///< Base aerodynamic drag coefficient.
    Ratio liftCoefficient = 1.93f;; ///< Base lift coefficient (proportional to v²).
    Ratio flyingBrakesDragRatio = 6.0f; ///< Drag multiplier when air brakes are deployed.
    Ratio flyingGearDragRatio = 1.8f; ///< Extra drag when gear is deployed in flight.
    Ratio groundBrakesDragRatio = 1000.0f; ///< Drag multiplier for wheel brakes on the ground.
    Ratio stallLiftRatio = 0.1f; ///< Lift reduction factor below stall speed.
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AircraftPhysicsConfig, weight, engineThrust, maxSpeed, stallSpeed, groundBrakesSpeed, dragCoefficient,
                                                liftCoefficient, flyingBrakesDragRatio, flyingGearDragRatio, groundBrakesDragRatio, stallLiftRatio);

class AircraftPhysics {
    // -- configuration (loaded from app.jsonc at init time) --
    AircraftPhysicsConfig conf;

public:
    explicit AircraftPhysics(const AppConfig &config);

    /// @brief Compute forces, integrate velocity/position, and apply ground clamping.
    /// @param state  Mutable aircraft state (forces, velocity, position updated in place).
    /// @param dt     Frame delta time (seconds).
    void update(AircraftState &state, float dt) const;
};
