#pragma once
#include "../json.hpp"

using json = nlohmann::json;

class AppConfig {
    json config;

public:
    AppConfig();

    // root
    std::string_view name() { return config["name"].get<std::string_view>(); }

    // config
    int screenWidth() { return config["config"]["screenWidth"].get<int>(); }
    int screenHeight() { return config["config"]["screenHeight"].get<int>(); }
    float clipPlans() { return config["config"]["clipPlans"].get<float>(); }

    // splash
    std::string_view splashBgPath() { return config["splash"]["bgPath"].get<std::string_view>(); }
    std::string_view splashMusicPath() { return config["splash"]["musicPath"].get<std::string_view>(); }
};
