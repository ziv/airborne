#pragma once
#include <fstream>
#include "raylib.h"
#include "../json.hpp"

using json = nlohmann::json;

class AppConfig {
    json config;

public:
    AppConfig() {
        std::ifstream file("app.json");
        if (!file.is_open()) {
            throw std::runtime_error("Could not open app.json");
        }
        file >> config;
        file.close();
        TraceLog(LOG_INFO, "[AppConfig] configuration loaded");
    }

    // root
    json raw() { return config; }
    std::string_view name() { return config["name"].get<std::string_view>(); }

    // config
    int screenWidth() { return config["config"]["screenWidth"].get<int>(); }
    int screenHeight() { return config["config"]["screenHeight"].get<int>(); }
    float clipPlans() { return config["config"]["clipPlans"].get<float>(); }

    // splash
    std::string_view splashBgPath() { return config["splash"]["bgPath"].get<std::string_view>(); }
    std::string_view splashMusicPath() { return config["splash"]["musicPath"].get<std::string_view>(); }

    // pilot
    float pilotFov() { return config["pilot"]["fov"].get<float>(); }
    float pilotTilt() { return config["pilot"]["tilt"].get<float>(); }

    // autopilot
    float maxBankAngle() { return config["autoPilot"]["maxBankAngle"].get<float>(); }
    float maxPullRatio() { return config["autoPilot"]["pullRatio"].get<float>(); }
    float speedRatio() { return config["autoPilot"]["speedRatio"].get<float>(); }

    // airplane
    float maxSpeed() { return config["airplane"]["maxSpeed"].get<float>(); }
    float minSpeed() { return config["airplane"]["minSpeed"].get<float>(); }
    float pitchRatio() { return config["airplane"]["pitchRatio"].get<float>(); }
    float rollRaio() { return config["airplane"]["rollRaio"].get<float>(); }
    float yawRatio() { return config["airplane"]["yawRatio"].get<float>(); }
    float acceleration() { return config["airplane"]["acceleration"].get<float>(); }
    float stallSpeed() { return config["airplane"]["stallSpeed"].get<float>(); }

    // development
    bool showGrid() { return config["development"]["showGrid"].get<bool>(); }
};
