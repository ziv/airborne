#pragma once
#include "raylib.h"
#include "../lib/json.hpp"

using json = nlohmann::json;

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
    float pilotFov;
    float pilotTilt;

    // autopilot
    float maxBankAngle;
    float maxPullRatio;
    float speedRatio;

    // aircraft
    float heightAboveGround;
    float weight;
    float maxSpeed;
    float minSpeed;
    float vleSpeed;
    float pitchRatio;
    float rollRatio;
    float yawRatio;
    float acceleration;
    float stallSpeed;
    float engineThrust;
    float liftCoefficient;
    float dragCoefficient;
    float bankInduceYawRatio;
    float liftLossPitchRatio;

    // development
    float showGrid = true;

    AppConfig();
};
