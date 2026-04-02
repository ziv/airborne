#pragma once
#include <string>
#include "../lib/json.hpp"

class AppConfig {
    [[nodiscard]] const nlohmann::json &getNode(const std::string &path) const;

public:
    nlohmann::json config;


    // todo remove all
    // std::string name;
    // // config
    // int screenWidth;
    // int screenHeight;
    // int clipPlans;
    //
    // // splash
    // std::string splashBgPath;
    // std::string splashMusicPath;
    //
    // // game
    // std::string gameCockpitTexture;
    // std::string gameCockpitChroma;
    // std::string gameEngineSound;
    // std::string gameMapTexture;
    // std::string gameMapHeightmap;
    // std::string gameMapMinimap;
    // float gameMapSizeX;
    // float gameMapSizeY;
    // float gameMapSizeZ;
    //
    // // pilot
    // Degree pilotFov;
    // Degree pilotTilt;
    //
    // // autopilot
    // float maxBankAngle;
    // float maxPullRatio;
    // float speedRatio;
    //
    // // aircraft
    // Meter heightAboveGround;
    // Newton weight;
    // Newton engineThrust;
    // MeterPerSecond maxSpeed;
    // MeterPerSecond minSpeed;
    // MeterPerSecond vleSpeed;
    // MeterPerSecond stallSpeed;
    // NoUint pitchRatio;
    // NoUint rollRatio;
    // NoUint yawRatio;
    // float acceleration;
    // float liftCoefficient;
    // float dragCoefficient;
    // float bankInduceYawRatio;
    // float liftLossPitchRatio;
    //
    // // development
    // bool showGrid = true;
    //
    // // views
    // std::string gaugeSprite;
    // std::string gaugeSpriteJson;

    AppConfig();

    /// @brief Fetch key by type
    /// @brief Key must start with "/" or an empty string for the whole tree
    template<typename T>
    T get(const std::string &path) const {
        return getNode(path).get<T>();
    }

    // /**
    //  * Fetch a string from the configuration by list of keys
    //  */
    // [[nodiscard]] std::string_view s(const std::string &path) const;
    //
    // /**
    //  * Fetch a C string from the configuration by list of keys
    //  */
    // [[nodiscard]] const char *c(const std::string &path) const;
    //
    // /**
    //  * Fetch an integer from the configuration by list of keys
    //  */
    // [[nodiscard]] int i(const std::string &path) const;
    //
    // /**
    //  * Fetch a float from the configuration by list of keys
    //  */
    // [[nodiscard]] float f(const std::string &path) const;
};

/**
 * The "inline" here is not by mistake
 * We added the same function again but with a return value instead of a
 * template and the linker will be very angry without this inline :)
 */
// template<>
// inline const char *AppConfig::get<const char *>(const std::string &path) const {
//     return getNode(path).get<std::string_view>().data();
// }
