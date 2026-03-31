#include "AppConfig.h"
#include "../primitives/Utils.h"

AppConfig::AppConfig() {
    const json config = UtilsLoaders::LoadJson("app.json");

    name = config["name"].get<std::string>();

    screenWidth = config["config"]["screenWidth"].get<int>();
    screenHeight = config["config"]["screenHeight"].get<int>();
    clipPlans = config["config"]["clipPlans"].get<int>();

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

    gaugeSprite = config["view"]["gaugeSprite"].get<std::string>();
    gaugeSprite = config["view"]["gaugeSpriteJson"].get<std::string>();
}
