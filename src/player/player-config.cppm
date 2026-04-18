module;
#include <nlohmann/json.hpp>
#include <entt/entt.hpp>
#include "../lib/ray.hpp"

export module Player:Config;

import Types;
import JsonConfig;

export struct PlayerCameraConfig {
    float tilt = 0.45f; ///< Downward pitch offset (radians) applied to the pilot's view.
    float fov = 85.0f;
};

export struct PlayerControlsConfig {
    float pitchRatio = 0.7f; ///< Pitch sensitivity (degrees per second at full deflection).
    float rollRatio = 0.9f; ///< Roll sensitivity.
    float yawRatio = 0.5f; ///< Yaw (rudder) sensitivity.
};

export struct PlayerPhysicsConfig {
    Newton weight = 120000.0f; ///< Aircraft empty weight (Newtons).
    Newton engineThrust = 130000.0f; ///< Maximum engine thrust at full military power (N).
    MeterPerSecond maxSpeed = 600.0f; ///< Hard speed cap (m/s) — safety limit.
    MeterPerSecond stallSpeed = 65.0f; ///< Speed below which lift drops sharply.
    // MeterPerSecond groundBrakesSpeed = 10.0f; ///< Speed threshold for ground braking damping.
    Ratio dragCoefficient = 0.36f; ///< Base aerodynamic drag coefficient.
    Ratio liftCoefficient = 1.93f;; ///< Base lift coefficient (proportional to v²).
    Ratio frictionCoefficient = 2.0f;
    // Ratio flyingBrakesDragRatio = 6.0f; ///< Drag multiplier when air brakes are deployed.
    // Ratio flyingGearDragRatio = 1.8f; ///< Extra drag when gear is deployed in flight.
    // Ratio groundBrakesDragRatio = 1000.0f; ///< Drag multiplier for wheel brakes on the ground.
    // Ratio stallLiftRatio = 0.1f; ///< Lift reduction factor below stall speed.
};

export struct PlayerPositionConfig {
    float threshold = 5000.0f;
    std::string heightPath = "assets/images/north-hm.png";
    float maxRelativeHeight = 5000.f;
    float heightMapSizeRatio = 125.0f; // ratio between the large area and the map we check the height
    float heightAboveGround = 3.f; // the pilot is not sitting on "0"
};

export struct PlayerTransformationConfig {
    MeterPerSecond maxSpeed = 600.0f; ///< Reference speed for normalising control authority.
    MeterPerSecond vleSpeed = 150.0f; ///< Max gear-extended speed; above this → turbulence.
    MeterPerSecond stallSpeed = 65.0f; ///< Below this speed aerodynamic effects diminish.
    Ratio bankInduceYawRatio = 0.2f; ///< Adverse yaw factor from bank angle.
    Ratio liftLossPitchRatio = 0.1f; ///< Nose-down pitch tendency when lift vector tilts.
};

export struct PlayerGroundCheckConfig {
    MeterPerSecond stallSpeed = 65.0f;
};

export
{
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerCameraConfig, tilt, fov);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerControlsConfig, pitchRatio, rollRatio, yawRatio);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerPhysicsConfig, weight, engineThrust, maxSpeed, stallSpeed, dragCoefficient, liftCoefficient, frictionCoefficient);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerPositionConfig, threshold);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerTransformationConfig, maxSpeed, vleSpeed, stallSpeed, bankInduceYawRatio, liftLossPitchRatio);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerGroundCheckConfig, stallSpeed);
}
