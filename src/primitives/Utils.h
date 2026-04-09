/**
 * @file Utils.h
 * @brief Utility functions: vector helpers, number formatting, and asset loaders.
 */
#pragma once
// todo clean all irrelevant include
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

/// @brief Project a 3D vector onto the horizontal (XZ) plane and normalise.
/// Falls back to the up vector's XZ projection when forward is nearly vertical.
inline Vector3 GetFlatForward(const Vector3 &currentForward, const Vector3 &currentUp) {
    Vector3 flatForward = {currentForward.x, 0.0f, currentForward.z};
    // edge case guard
    if (Vector3Length(flatForward) < 0.001f) {
        flatForward = {currentUp.x, 0.0f, currentUp.z};
    }
    return Vector3Normalize(flatForward);
}

/// @brief Compute a right-hand perpendicular from a forward vector on the XZ plane.
inline Vector3 GetFlatRight(const Vector3 &currentForward, const Vector3 &currentUp = GamePhysics::WorldUp) {
    return Vector3Normalize(Vector3CrossProduct(currentForward, currentUp));
}

/// @brief Format a number with K/M suffix for HUD display (std::string version).
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

/// @brief Format a number with K/M suffix for HUD display (C-string version via TextFormat).
inline const char *FormatNumberSuffix(const float number) {
    if (number >= 1000000.0f) {
        return TextFormat("%.1fM", number / 1000000.0f);
    }
    if (number >= 1000.0f) {
        return TextFormat("%.1fk", number / 1000.0f);
    }
    return TextFormat("%.0f", number);
}


/// @brief Asset loading utilities.
namespace UtilsLoaders {
    /// @brief Build a terrain Model from a texture image and a heightmap image.
    /// @param texturePath   Path to the satellite/color texture.
    /// @param heightmapPath Path to the greyscale heightmap.
    /// @param size          World-space dimensions {width, height, depth} in meters.
    /// @return A raylib Model with the generated mesh and assigned texture.
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

    /// @brief Load a JSONC file (JSON with comments) and return the parsed tree.
    /// @param path Filesystem path to the .jsonc file.
    /// @throws std::runtime_error if the file cannot be opened.
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
