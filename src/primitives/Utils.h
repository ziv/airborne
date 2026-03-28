#pragma once
#include <iostream>
#include <fstream>

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

inline Model TmpLoadModel() {
    Image textureImage = LoadImage("res/texture.png");
    const Texture2D texture = LoadTextureFromImage(textureImage);
    UnloadImage(textureImage);

    Image heightImage = LoadImage("res/heightmap.png");
    // Vector3 size = {10000.0f, 10000.0f, 10000.0f};
    Vector3 size = {100000.0f, 20000.0f, 100000.0f};
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
    const float engineThrust = game.throttle * 150;
    const float currentSpeed = game.Speed();
    Vector3 velocity = game.velocity;

    // forces

    // gravity
    constexpr Vector3 gravityForce = { 0.0f, -gravity, 0.0f };

    // thrust
    const Vector3 thrustForce = Vector3Scale(game.GetForward(), engineThrust);

    // lift magnitude
    constexpr float liftCoefficient = 0.0015f; // שחק עם המספר הזה עד שתרגיש שהמטוס טס טוב
    float liftMagnitude = (currentSpeed * currentSpeed) * liftCoefficient;

    // stall
    bool isStalling = false;
    if (currentSpeed < config.stallSpeed()) {
        // too slow, cut the lift by 90%
        liftMagnitude *= 0.1f;
        isStalling = true;
    }

    // lift force
    const Vector3 liftForce = Vector3Scale(game.GetUp(), liftMagnitude);

    // combining all forces
    Vector3 totalForce = Vector3Add(Vector3Add(gravityForce, thrustForce), liftForce);

    // acceleration results
    velocity = Vector3Add(velocity, Vector3Scale(totalForce, game.deltaTime));


    // drag
    velocity = Vector3Scale(velocity, 0.99f);

    // weathervaning
    if (currentSpeed > 5.0f && !isStalling) {
        auto [x, y, z] = Vector3Scale(game.GetForward(), currentSpeed);
        velocity.x = Lerp(velocity.x, x, 2.0f * game.deltaTime);
        velocity.y = Lerp(velocity.y, y, 2.0f * game.deltaTime);
        velocity.z = Lerp(velocity.z, z, 2.0f * game.deltaTime);
    }

    return velocity;
}