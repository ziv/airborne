#pragma once
#include "Primitives.h"
#include "../lib/json.hpp"

using json = nlohmann::json;

/**
*struct AircraftProperties {
Newton weight;
Newton engineThrust;
MeterPerSecond maxSpeed;
MeterPerSecond stallSpeed;
MeterPerSecond groundBrakesSpeed;
Ratio dragCoefficient;
Ratio liftCoefficient;

float bankInduceYawRatio;
float liftLossPitchRatio;
};
*/

// const nlohmann::json &AppConfig::getNode(const std::string &path) const {
//     try {
//         return config.at(nlohmann::json::json_pointer(path));
//     } catch (const nlohmann::json::exception &e) {
//         TraceLog(LOG_ERROR, TextFormat("Config error, missing or invalid path: %s, reason: %s", path.data(), e.what()));
//         throw std::runtime_error("Config path not found: " + path);
//     }
// }

/// @brief Takes a JSON object and extract AircraftProperties
inline AircraftProperties extractProperties(const json &data) {
    return (AircraftProperties){
        data.at("/weight"_json_pointer).get<float>(),
        data.at("/engineThrust"_json_pointer).get<float>(),
        data.at("/maxSpeed"_json_pointer).get<float>(),
        data.at("/stallSpeed"_json_pointer).get<float>(),
        data.at("/groundBrakesSpeed"_json_pointer).get<float>(),
        data.at("/dragCoefficient"_json_pointer).get<float>(),
        data.at("/liftCoefficient"_json_pointer).get<float>(),
    };
}
