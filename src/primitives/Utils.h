#pragma once
// todo clean all irelevant include
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>
#include "raylib.h"
#include "raymath.h"
#include "Constants.h"
#include <string_view>
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

// todo replace with somthing better
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


namespace UtilsLoaders {
    inline Model loadTerrain(std::string_view texturePath, std::string_view heightmapPath, const Vector3 size) {
        const Image textureImage = LoadImage(texturePath.data());
        const Texture2D texture = LoadTextureFromImage(textureImage);
        UnloadImage(textureImage);

        const Image heightImage = LoadImage(heightmapPath.data());
        // const Vector3 size = {10000.0f, 5000.0f, 10000.0f};
        // Vector3 size = {100000.0f, 20000.0f, 100000.0f};
        const Mesh mesh = GenMeshHeightmap(heightImage, size);
        UnloadImage(heightImage);

        const Model model = LoadModelFromMesh(mesh);
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
        return model;
    }

    inline json LoadJson(const std::string &path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            TraceLog(LOG_ERROR, TextFormat("[LoadJson] unable to open file: %s", path.c_str()));
            throw std::runtime_error("[LoadJson] unable to open file");
        }
        json data = json::parse(file, nullptr, true, true);
        file.close();
        TraceLog(LOG_INFO, TextFormat("[LoadJson] file %s loaded", path.c_str()));
        return data;
    }
}
