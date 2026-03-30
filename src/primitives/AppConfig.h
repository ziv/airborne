#pragma once
#include "raylib.h"
#include "../lib/json.hpp"

using json = nlohmann::json;

class AppConfig {
    json config;

public:
    AppConfig();

    // root
    json raw() { return config; }
    std::string_view name() { return config["name"].get<std::string_view>(); }

    // config
    int screenWidth() { return config["config"]["screenWidth"].get<int>(); }
    int screenHeight() { return config["config"]["screenHeight"].get<int>(); }
    float clipPlans() { return config["config"]["clipPlans"].get<float>(); }

    // splash screen
    std::string_view splashBgPath() { return config["screens"]["splash"]["bgPath"].get<std::string_view>(); }
    std::string_view splashMusicPath() { return config["screens"]["splash"]["musicPath"].get<std::string_view>(); }

    // game screen
    std::string_view gameCockpitTexture() { return config["screens"]["game"]["cockpitTexture"].get<std::string_view>(); }

    std::string_view gameCockpitChroma() { return config["screens"]["game"]["cockpitChroma"].get<std::string_view>(); }
    std::string_view gameEngineSound() { return config["screens"]["game"]["engineSound"].get<std::string_view>(); }
    std::string_view gameMapTexture() { return config["screens"]["game"]["mapTexture"].get<std::string_view>(); }
    std::string_view gameMapHeightmap() { return config["screens"]["game"]["mapHeightmap"].get<std::string_view>(); }
    float gameMapSizeX() { return config["screens"]["game"]["mapSizeX"].get<float>(); }
    float gameMapSizeY() { return config["screens"]["game"]["mapSizeY"].get<float>(); }
    float gameMapSizeZ() { return config["screens"]["game"]["mapSizeZ"].get<float>(); }

    // pilot
    float pilotFov() { return config["pilot"]["fov"].get<float>(); }
    float pilotTilt() { return config["pilot"]["tilt"].get<float>(); }

    // autopilot
    float maxBankAngle() { return config["autoPilot"]["maxBankAngle"].get<float>(); }
    float maxPullRatio() { return config["autoPilot"]["pullRatio"].get<float>(); }
    float speedRatio() { return config["autoPilot"]["speedRatio"].get<float>(); }

    // airplane
    float heightAboveGround() { return config["airplane"]["heightAboveGround"].get<float>(); }
    float weight() { return config["airplane"]["weight"].get<float>(); }
    float maxSpeed() { return config["airplane"]["maxSpeed"].get<float>(); }
    float minSpeed() { return config["airplane"]["minSpeed"].get<float>(); }
    float pitchRatio() { return config["airplane"]["pitchRatio"].get<float>(); }
    float rollRaio() { return config["airplane"]["rollRaio"].get<float>(); }
    float yawRatio() { return config["airplane"]["yawRatio"].get<float>(); }
    float acceleration() { return config["airplane"]["acceleration"].get<float>(); }
    float stallSpeed() { return config["airplane"]["stallSpeed"].get<float>(); }
    float engineThrust() { return config["airplane"]["engineThrust"].get<float>(); }
    float liftCoefficient() { return config["airplane"]["liftCoefficient"].get<float>(); }
    float dragCoefficient() { return config["airplane"]["dragCoefficient"].get<float>(); }
    float bankInduceYawRatio() { return config["airplane"]["bankInduceYawRatio"].get<float>(); }
    float liftLossPitchRatio() { return config["airplane"]["liftLossPitchRatio"].get<float>(); }

    // development
    bool showGrid() { return config["development"]["showGrid"].get<bool>(); }
};
