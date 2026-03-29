#pragma once
#include "raylib.h"
#include <string_view>

namespace UtilsLoaders {
    inline Model loadTerrain(std::string_view texturePath, std::string_view heightmapPath, const Vector3 size)
    {
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
}
