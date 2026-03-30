#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>
#include "GameData.h"
#include "raylib.h"
#include "raymath.h"
#include "../Constants.h"
#include "../lib/json.hpp"

using json = nlohmann::json;

inline Vector3 GetFlatForward(const Vector3 &currentForward, const Vector3 &currentUp) {
    Vector3 flatForward = {currentForward.x, 0.0f, currentForward.z};
    // edge case guard
    if (Vector3Length(flatForward) < 0.001f) {
        flatForward = {currentUp.x, 0.0f, currentUp.z};
    }
    return Vector3Normalize(flatForward);
}

inline Vector3 GetFlatRight(const Vector3 &currentForward, const Vector3 &currentUp = GamePhysics::WorldUp) {
    return Vector3Normalize(Vector3CrossProduct(currentForward, currentUp));
}

inline std::string FormatNumber(const float num) {
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

// inline Model TmpLoadModel() {
//     Image textureImage = LoadImage("res/texture-il.jpg");
//     const Texture2D texture = LoadTextureFromImage(textureImage);
//     UnloadImage(textureImage);
//
//     Image heightImage = LoadImage("res/heightmap-il.png");
//     Vector3 size = {2048.0f, 2788.0f, 2048.0f};
//     // Vector3 size = {100000.0f, 20000.0f, 100000.0f};
//     Mesh mesh = GenMeshHeightmap(heightImage, size);
//     UnloadImage(heightImage);
//
//     Model model = LoadModelFromMesh(mesh);
//     model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
//     return model;
// }


inline json LoadJson(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("[LoadJson] Could not open app.json");
    }
    json data;
    file >> data;
    file.close();
    TraceLog(LOG_INFO, TextFormat("[LoadJson] file %s loaded", path.c_str()));
    return data;
}

// inline json LoadAppConfig() {
//     std::ifstream file("app.json");
//     if (!file.is_open()) {
//         throw std::runtime_error("Could not open app.json");
//     }
//     json data;
//     file >> data;
//     file.close();
//     TraceLog(LOG_INFO, "app.json loaded");
//     return data;
// }
//
// constexpr float gravity = 9.81f;

// inline Vector3 UpdatePhysics(const GameData &game, AppConfig &config) {
//     const float engineThrust = game.throttle * config.engineThrust();
//     const float currentSpeed = game.Speed();
//     const auto negativeSpeed = currentSpeed == 0.0f ? 1.0f : 1 / currentSpeed;
//     Vector3 velocity = game.velocity;
//
//     // collect all forces
//
//     // gravity
//     constexpr Vector3 gravityForce = {0.0f, -gravity, 0.0f};
//
//     // thrust
//     const auto thrustForce = Vector3Scale(game.GetForward(), engineThrust);
//
//     // lift magnitude (depends on speed^2 and wingspan represented as lift coefficien
//     auto liftMagnitude = (currentSpeed * currentSpeed) * config.liftCoefficient();
//
//     // stall
//     bool isStalling = false;
//     if (currentSpeed < config.stallSpeed()) {
//         // too slow, cut the lift by 90%
//         liftMagnitude *= 0.1f;
//         isStalling = true;
//     }
//
//     // lift force
//     const auto liftForce = Vector3Scale(game.GetUp(), liftMagnitude);
//
//     // drag force
//     const auto dragMagnitude = currentSpeed * config.dragCoefficient();
//     const auto dragForce = Vector3Scale(game.GetForward(), -dragMagnitude);
//
//     // combining all forces
//     const auto totalForce = Vector3Add(Vector3Add(Vector3Add(thrustForce, dragForce), gravityForce), liftForce);
//
//     // acceleration results
//     velocity = Vector3Add(velocity, Vector3Scale(totalForce, game.deltaTime));
//
//
//     // weathervaning
//     if (!isStalling) {
//         auto [x, y, z] = Vector3Scale(game.GetForward(), currentSpeed);
//         velocity.x = Lerp(velocity.x, x, 2.0f * game.deltaTime);
//         velocity.y = Lerp(velocity.y, y, 2.0f * game.deltaTime);
//         velocity.z = Lerp(velocity.z, z, 2.0f * game.deltaTime);
//     }
//
//     // limit velocity
//     while (Vector3Length(velocity) > config.maxSpeed()) {
//         velocity = Vector3Scale(velocity, 0.9f);
//     }
//
//     return velocity;
// }
