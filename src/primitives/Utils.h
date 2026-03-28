#pragma once
#include <iostream>
#include <fstream>
#include "raylib.h"
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