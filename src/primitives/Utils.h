#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>

#include "GameData.h"
#include "raylib.h"
#include "raymath.h"
#include "../json.hpp"

using json = nlohmann::json;

inline Vector3 GetFlatForward(const Vector3 &currentForward, const Vector3 &currentUp) {
    Vector3 flatForward = {currentForward.x, 0.0f, currentForward.z};
    // edge case guard
    if (Vector3Length(flatForward) < 0.001f) {
        flatForward = {currentUp.x, 0.0f, currentUp.z};
    }
    return flatForward;
}

inline Vector3 GetFlatRight(const Vector3 &currentForward, const Vector3 &currentUp = GamePhysics::WorldUp) {
    return Vector3Normalize(Vector3CrossProduct(currentForward, currentUp));
}

inline std::string formatNumber(const float num) {
    std::stringstream ss;
    if (num >= 1000000) {
        ss << std::fixed << std::setprecision(2) << (num / 1000000.0) << "M";
    } else if (num >= 1000) {
        ss << std::fixed << std::setprecision(2) << (num / 1000.0) << "K";
    } else {
        ss << num;
    }
    return ss.str();
}

inline Model TmpLoadModel() {
    Image textureImage = LoadImage("res/texture.jpg");
    const Texture2D texture = LoadTextureFromImage(textureImage);
    UnloadImage(textureImage);

    Image heightImage = LoadImage("res/heightmap.png");
    Vector3 size = {10000.0f, 5000.0f, 10000.0f};
    // Vector3 size = {100000.0f, 20000.0f, 100000.0f};
    Mesh mesh = GenMeshHeightmap(heightImage, size);
    UnloadImage(heightImage);

    Model model = LoadModelFromMesh(mesh);
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    return model;
}

inline json LoadAppConfig() {
    std::ifstream file("app.json");
    if (!file.is_open()) {
        throw std::runtime_error("Could not open app.json");
    }
    json data;
    file >> data;
    file.close();
    TraceLog(LOG_INFO, "app.json loaded");
    return data;
}

constexpr float gravity = 9.81f;

// todo breaks on/off, zero height, landing
inline Vector3 UpdatePhysics(GameData &game, AppConfig &config) {
    const float engineThrust = game.throttle * config.engineThrust();
    const float currentSpeed = game.Speed();
    Vector3 velocity = game.velocity;

    // collect all forces

    // gravity
    constexpr Vector3 gravityForce = {0.0f, -gravity, 0.0f};

    // thrust
    const Vector3 thrustForce = Vector3Scale(game.GetForward(), engineThrust);

    // lift magnitude
    float liftMagnitude = (currentSpeed * currentSpeed) * config.liftCoefficient();

    // stall
    bool isStalling = false;
    if (currentSpeed < config.stallSpeed()) {
        // too slow, cut the lift by 99%
        liftMagnitude *= 0.01f;
        isStalling = true;
    }

    // lift force
    const Vector3 liftForce = Vector3Scale(game.GetUp(), liftMagnitude);

    // combining all forces
    const Vector3 totalForce = Vector3Add(Vector3Add(gravityForce, thrustForce), liftForce);

    // acceleration results
    velocity = Vector3Add(velocity, Vector3Scale(totalForce, game.deltaTime));

    // drag
    const float dragCoefficient = config.dragCoefficient();
    float dragFactor = 1.0f - (dragCoefficient * game.deltaTime);
    if (dragFactor < 0.0f) dragFactor = 0.0f;
    velocity = Vector3Scale(velocity, dragFactor);

    // weathervaning
    if (!isStalling) {
        auto [x, y, z] = Vector3Scale(game.GetForward(), currentSpeed);
        velocity.x = Lerp(velocity.x, x, 2.0f * game.deltaTime);
        velocity.y = Lerp(velocity.y, y, 2.0f * game.deltaTime);
        velocity.z = Lerp(velocity.z, z, 2.0f * game.deltaTime);
    }

    // limit velocity
    while (Vector3Length(velocity) > 600.0f) {
        velocity = Vector3Scale(velocity, 0.9f);
    }
    return velocity;
}
