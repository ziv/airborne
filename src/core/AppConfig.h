#pragma once
#include <string>
#include "../primitives/Types.h"

class AppConfig {
public:
    std::string name;
    // config
    int screenWidth;
    int screenHeight;
    int clipPlans;

    // splash
    std::string splashBgPath;
    std::string splashMusicPath;

    // game
    std::string gameCockpitTexture;
    std::string gameCockpitChroma;
    std::string gameEngineSound;
    std::string gameMapTexture;
    std::string gameMapHeightmap;
    std::string gameMapMinimap;
    float gameMapSizeX;
    float gameMapSizeY;
    float gameMapSizeZ;

    // pilot
    Degree pilotFov;
    Degree pilotTilt;

    // autopilot
    float maxBankAngle;
    float maxPullRatio;
    float speedRatio;

    // aircraft
    Meter heightAboveGround;
    Newton weight;
    Newton engineThrust;
    MeterPerSecond maxSpeed;
    MeterPerSecond minSpeed;
    MeterPerSecond vleSpeed;
    MeterPerSecond stallSpeed;
    NoUint pitchRatio;
    NoUint rollRatio;
    NoUint yawRatio;
    float acceleration;
    float liftCoefficient;
    float dragCoefficient;
    float bankInduceYawRatio;
    float liftLossPitchRatio;

    // development
    bool showGrid = true;

    AppConfig();
};
