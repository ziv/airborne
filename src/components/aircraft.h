#pragma once
#include "../primitives/Types.h"

struct Aircraft {
    bool gear;
    bool brakes;
};

struct Aerodynamics {
    Newton weight;
    Ratio liftCoefficient;
    Ratio dragCoefficient;
    Ratio brakesDragCoefficient;
    Ratio gearDragCoefficient;
};

struct Engine {
    float maxThrust;
    float currentThrottle;
};

struct Hud {
    int ladderX;
    int ladderY;
    int ladderWidth;
    int ladderHeight;
    float ladderOffset;
    // float fov;
    // int width;
    // int height;
};

//
// struct AircraftProperties {
//     Newton weight = 120000.0f; ///< Aircraft empty weight (Newtons).
//     Newton engineThrust = 130000.0f; ///< Maximum engine thrust at full military power (N).
//     MeterPerSecond maxSpeed = 600.0f; ///< Hard speed cap (m/s) — safety limit.
//     MeterPerSecond stallSpeed = 65.0f; ///< Speed below which lift drops sharply.
//
//     Ratio liftCoefficient = 1.93f;; ///< Base lift coefficient (proportional to v²).
//
//     Ratio dragCoefficient = 0.36f; ///< Base aerodynamic drag coefficient.
//     Ratio airBrakesDragCoefficient = 0.36f;
//     Ratio gearDragCoefficient = 0.36f;
//
//
//
//     // MeterPerSecond groundBrakesSpeed = 10.0f; ///< Speed threshold for ground braking damping.
//     //
//     // Ratio flyingBrakesDragRatio = 6.0f; ///< Drag multiplier when air brakes are deployed.
//     // Ratio flyingGearDragRatio = 1.8f; ///< Extra drag when gear is deployed in flight.
//     // Ratio groundBrakesDragRatio = 1000.0f; ///< Drag multiplier for wheel brakes on the ground.
//     // Ratio stallLiftRatio = 0.1f; ///< Lift reduction factor below stall speed.
// };
