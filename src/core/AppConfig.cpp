#include "AppConfig.h"
#include "../primitives/Utils.h"

const nlohmann::json &AppConfig::getNode(const std::string &path) const {
    try {
        return config.at(nlohmann::json::json_pointer(path));
    } catch (const nlohmann::json::exception &e) {
        std::cerr << "[Config Error] Missing or invalid path: "
                << path
                << "\n"
                << "Reason: "
                << e.what()
                << "\n";
        throw std::runtime_error("Config path not found: " + path);
    }
}

AppConfig::AppConfig() : config(UtilsLoaders::LoadJson("config/app.jsonc")) {
    name = config["name"].get<std::string>();

    screenWidth = config["config"]["screenWidth"].get<int>();
    screenHeight = config["config"]["screenHeight"].get<int>();
    // clipPlans = config["config"]["clipPlans"].get<int>();

    splashBgPath = config["splash"]["bgPath"].get<std::string>();
    splashMusicPath = config["splash"]["musicPath"].get<std::string>();

    gameCockpitTexture = config["game"]["cockpitTexture"].get<std::string>();
    gameCockpitChroma = config["game"]["cockpitChroma"].get<std::string>();
    gameEngineSound = config["game"]["engineSound"].get<std::string>();
    gameMapTexture = config["game"]["mapTexture"].get<std::string>();
    gameMapHeightmap = config["game"]["mapHeightmap"].get<std::string>();
    gameMapSizeX = config["game"]["mapSizeX"].get<float>();
    gameMapSizeY = config["game"]["mapSizeY"].get<float>();
    gameMapSizeZ = config["game"]["mapSizeZ"].get<float>();

    pilotFov = config["pilot"]["fov"].get<float>();
    pilotTilt = config["pilot"]["tilt"].get<float>();

    maxBankAngle = config["autoPilot"]["maxBankAngle"].get<float>();
    maxPullRatio = config["autoPilot"]["pullRatio"].get<float>();
    speedRatio = config["autoPilot"]["speedRatio"].get<float>();

    heightAboveGround = config["airplane"]["heightAboveGround"].get<float>();
    weight = config["airplane"]["weight"].get<float>();
    maxSpeed = config["airplane"]["maxSpeed"].get<float>();
    minSpeed = config["airplane"]["minSpeed"].get<float>();
    vleSpeed = config["airplane"]["vleSpeed"].get<float>();
    pitchRatio = config["airplane"]["pitchRatio"].get<float>();
    rollRatio = config["airplane"]["rollRatio"].get<float>();
    yawRatio = config["airplane"]["yawRatio"].get<float>();
    acceleration = config["airplane"]["acceleration"].get<float>();
    stallSpeed = config["airplane"]["stallSpeed"].get<float>();
    engineThrust = config["airplane"]["engineThrust"].get<float>();
    liftCoefficient = config["airplane"]["liftCoefficient"].get<float>();
    dragCoefficient = config["airplane"]["dragCoefficient"].get<float>();
    bankInduceYawRatio = config["airplane"]["bankInduceYawRatio"].get<float>();
    liftLossPitchRatio = config["airplane"]["liftLossPitchRatio"].get<float>();

    showGrid = config["development"]["showGrid"].get<bool>();

    gaugeSprite = config["views"]["gaugeSprite"].get<std::string>();
    gaugeSprite = config["views"]["gaugeSpriteJson"].get<std::string>();
}

std::string_view AppConfig::s(const std::string &path) const {
    return getNode(path).get<std::string_view>();
}

const char *AppConfig::c(const std::string &path) const {
    return getNode(path).get<std::string_view>().data();
}

int AppConfig::i(const std::string &path) const {
    return getNode(path).get<int>();
}

float AppConfig::f(const std::string &path) const {
    return getNode(path).get<float>();
}
