/**
 * @file SceneManager.cpp
 * @brief Terrain rendering, ground-height sampling, fog, clouds, and engine audio.
 */
#include "SceneManager.h"

#include "rlgl.h"
#include "../primitives/Utils.h"

/**
 * @brief Generate a procedural cloud alpha texture from Perlin noise.
 *
 * Pixels below an intensity threshold become fully transparent (clear sky);
 * brighter pixels become semi-opaque white (clouds). The resulting texture
 * is uploaded to the GPU and the CPU image is freed.
 */
inline Texture2D initClouds() {
    // noise
    Image noiseImage = GenImagePerlinNoise(1024, 1024, 100, 100, 4.0f);

    // allow opacity
    ImageFormat(&noiseImage, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    // part of the pixels become clear and part ones become clouds (edit pixels in place)
    auto *pixels = static_cast<Color *>(noiseImage.data);

    for (int i = 0; i < noiseImage.width * noiseImage.height; i++) {
        if (const unsigned char intensity = pixels[i].r; intensity < 120) {
            // clear sky
            pixels[i] = {255, 255, 255, 0}; // לבן, אבל שקוף לחלוטין
        } else {
            // clouds
            const auto alpha = static_cast<unsigned char>((static_cast<float>(intensity) - 120) * 1.8f);
            pixels[i] = {255, 255, 255, alpha};
        }
    }

    const auto cloudTexture = LoadTextureFromImage(noiseImage);
    UnloadImage(noiseImage);
    return cloudTexture;
}

SceneManager::SceneManager(const AppConfig &config) : engineSound(LoadMusicStream(config.get<std::string_view>("/game/engineSound").data())),
                                                      // load as image (RAM/CPU) for height check
                                                      height(LoadImage(config.get<std::string_view>("/game/mapHeightmap").data())),
                                                      // load as texture (VRAM/GPU)
                                                      map(UtilsLoaders::loadTerrain(
                                                          config.get<std::string_view>("/game/mapTexture").data(),
                                                          config.get<std::string_view>("/game/mapHeightmap").data(),
                                                          {
                                                              config.get<float>("/game/mapSizeX"),
                                                              config.get<float>("/game/mapSizeY"),
                                                              config.get<float>("/game/mapSizeZ")
                                                          }
                                                      )),
                                                      relativeHeight(config.get<float>("/game/mapSizeY")),
                                                      fog(LoadShader(config.get<std::string_view>("/game/fogShaderVs").data(),
                                                                     config.get<std::string_view>("/game/fogShaderFs").data())),
                                                      cloudModel(LoadModelFromMesh(GenMeshPlane(200000, 200000, 10, 10))),
                                                      cloudTexture(initClouds()) {
    const int skyColorLoc = GetShaderLocation(fog, "skyColor");
    const int fogNearLoc = GetShaderLocation(fog, "fogNear");
    const int fogFarLoc = GetShaderLocation(fog, "fogFar");

    constexpr float fogNearValue = 45000.0f; // start at
    constexpr float fogFarValue = 80000.0f; // full fogs
    constexpr Vector3 skyColorVec = {BLUE.r / 255.0f, BLUE.g / 255.0f, BLUE.b / 255.0f};

    SetShaderValue(fog, skyColorLoc, &skyColorVec, SHADER_UNIFORM_VEC3);
    SetShaderValue(fog, fogNearLoc, &fogNearValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(fog, fogFarLoc, &fogFarValue, SHADER_UNIFORM_FLOAT);
    map->materials[0].shader = fog;

    PlayMusicStream(engineSound);

    // cloud
    SetTextureWrap(cloudTexture, TEXTURE_WRAP_REPEAT);
    cloudModel->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = cloudTexture;
    cloudModel->materials[0].shader = fog;
}

void SceneManager::update(AircraftState &state, float dt) {
    // bg sound
    const auto throttle = state.controls.throttle;
    if (throttle > 0) {
        const float targetPitch = 0.8f + (throttle * 0.7f);
        const float targetVolume = 0.2f + (throttle * 0.9f);
        SetMusicPitch(engineSound, targetPitch);
        SetMusicVolume(engineSound, targetVolume);
        UpdateMusicStream(engineSound);
    }

    // update ground height
    // todo should come from configuration
    // world size 128,000. map size 1,024. 128,000/1,024=125.0f
    const auto x = static_cast<int>((state.position.x - state.mapOffset.x) / 125.0f);
    const auto z = static_cast<int>((state.position.z - state.mapOffset.y) / 125.0f);
    if (x < 0 || z < 0 || x >= height->width || z >= height->height) {
        state.groundHeight = 0.0;
    } else {
        const auto r = static_cast<float>(GetImageColor(height, x, z).r);
        state.groundHeight = relativeHeight * r / 255.0f;
    }
    // tiles.update(state.position);
    cloudModel->materials[0].maps[MATERIAL_MAP_DIFFUSE].color.r += 0.01f * GetFrameTime();
}


void SceneManager::draw(const AircraftState &state, const Camera &camera) {
    const Vector3 drawPosition = {
        state.mapOffset.x,
        0.0f,
        state.mapOffset.y,
    };
    DrawModel(map, drawPosition, 1.0f, WHITE);

    // clouds
    const Vector3 cloudPosition = {
        state.mapOffset.x,
        10000.0f,
        state.mapOffset.y
    };
    rlDisableBackfaceCulling();
    DrawModel(cloudModel, cloudPosition, 1.0f, WHITE);
    rlEnableBackfaceCulling();
}
