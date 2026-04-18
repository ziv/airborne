module;
#include <entt/entt.hpp>
#include "../lib/ray.hpp"

export module Prefabs:Scene;

import JsonConfig;
import Components;
import ResourceManager;
import Types;

Model generate_world_model(const std::string &texture_path, const std::string &heightmap_path, const Vector3 &size, const Shader &fog) {
    // our world texture
    const Image textureImage = LoadImage(texture_path.c_str());
    const Texture2D texture = LoadTextureFromImage(textureImage);
    UnloadImage(textureImage);

    // our world heightmap
    const Image heightImage = LoadImage(heightmap_path.data());
    const Mesh mesh = GenMeshHeightmap(heightImage, size);
    UnloadImage(heightImage);

    // our world model
    const Model model = LoadModelFromMesh(mesh);
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    model.materials[0].shader = fog;

    return model;
}

/**
 * @brief Generate a procedural cloud alpha texture from Perlin noise.
 *
 * Pixels below an intensity threshold become fully transparent (clear sky);
 * brighter pixels become semi-opaque white (clouds). The resulting texture
 * is uploaded to the GPU and the CPU image is freed.
 */
Model init_clouds(const Shader &fog) {
    // noise
    Image noise_image = GenImagePerlinNoise(1024, 1024, 100, 100, 4.0f);

    // allow opacity
    ImageFormat(&noise_image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    // part of the pixels become clear and part ones become clouds (edit pixels in place)
    auto *pixels = static_cast<Color *>(noise_image.data);

    for (int i = 0; i < noise_image.width * noise_image.height; i++) {
        if (const unsigned char intensity = pixels[i].r; intensity < 150) {
            // clear sky
            pixels[i] = {255, 255, 255, 0}; // white, but fully transparent
        } else {
            // clouds
            const auto alpha = static_cast<unsigned char>((static_cast<float>(intensity) - 150) * 1.8f);
            pixels[i] = {255, 255, 255, alpha};
        }
    }

    const auto cloud_texture = LoadTextureFromImage(noise_image);
    UnloadImage(noise_image);

    const auto clouds_model = LoadModelFromMesh(GenMeshPlane(200000, 200000, 10, 10));
    clouds_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = cloud_texture;
    clouds_model.materials[0].shader = fog;
    return clouds_model;
}

Shader init_fog(const std::string &vs_path, const std::string &fs_path) {
    const auto fog = LoadShader(vs_path.c_str(), fs_path.c_str());

    const int skyColorLoc = GetShaderLocation(fog, "skyColor");
    const int fogNearLoc = GetShaderLocation(fog, "fogNear");
    const int fogFarLoc = GetShaderLocation(fog, "fogFar");

    constexpr float fogNearValue = 20000.0f; // start at
    constexpr float fogFarValue = 40000.0f; // full fogs
    constexpr Vector3 skyColorVec = {BLUE.r / 255.0f, BLUE.g / 255.0f, BLUE.b / 255.0f};

    SetShaderValue(fog, skyColorLoc, &skyColorVec, SHADER_UNIFORM_VEC3);
    SetShaderValue(fog, fogNearLoc, &fogNearValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(fog, fogFarLoc, &fogFarValue, SHADER_UNIFORM_FLOAT);

    return fog;
}

export namespace factories {
    entt::entity create_scene(entt::registry &registry,
                              const JsonConfig &config) {
        const auto [mapTexture, mapHeightmap, mapSize, fogShaderVs, fogShaderFs] = config.get<SceneConfig>("/scene");
        auto &assets = get_resource_manager(registry);

        constexpr auto fog_id = entt::hashed_string("fog_shader");
        Shader fog = init_fog(fogShaderVs, fogShaderFs);
        if (!assets.shaders.contains(fog_id)) assets.shaders.load(fog_id, fog);

        constexpr auto surface_id = entt::hashed_string("world_model");
        if (!assets.models.contains(surface_id)) {
            assets.models.load(surface_id, generate_world_model(mapTexture, mapHeightmap, mapSize, fog));
        }

        constexpr auto cloud_id = entt::hashed_string("cloud_model");
        if (!assets.models.contains(cloud_id)) {
            assets.models.load(cloud_id, init_clouds(fog));
        }

        // todo path from config
        constexpr auto engine_id = entt::hashed_string("assets/sound/engine.mp3");
        if (assets.music_streams.contains(engine_id)) {
            PlayMusicStream(assets.music_streams[engine_id]->res);
        } else {
            TraceLog(LOG_WARNING, "Engine sound resource not found for path 'assets/sound/engine.mp3'");
        }

        const auto entity = registry.create();

        registry.emplace<World>(entity,
                                assets.models[surface_id],
                                assets.models[cloud_id],
                                assets.music_streams[engine_id]);
        registry.emplace<Position3D>(entity, Vector3Zero(), Vector3Zero());

        return entity;
    }
}
