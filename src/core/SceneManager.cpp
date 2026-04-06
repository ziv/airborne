#include "SceneManager.h"
#include "../primitives/Utils.h"

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
                                                      relativeHeight(config.get<float>("/game/mapSizeY")) {
    // fogShader = LoadShader("shaders/fog.vs", "shaders/fog.fs");
    // camPosLoc = GetShaderLocation(fogShader, "cameraPos");
    // fogColorLoc = GetShaderLocation(fogShader, "fogColor");
    // fogDensityLoc = GetShaderLocation(fogShader, "fogDensity");
    // float fogColorNormalized[4] = {
    //     CurrentFogColor.r / 255.0f,
    //     CurrentFogColor.g / 255.0f,
    //     CurrentFogColor.b / 255.0f,
    //     CurrentFogColor.a / 255.0f
    // };
    // SetShaderValue(fogShader, fogColorLoc, fogColorNormalized, SHADER_UNIFORM_VEC4);
    // SetShaderValue(fogShader, fogDensityLoc, &CurrentFogDensity, SHADER_UNIFORM_FLOAT);
    PlayMusicStream(engineSound);
}

// SceneManager::~SceneManager() {
//     // UnloadModel(mig);
// }

void SceneManager::update(AircraftState &state, float dt) {
    // bg sound
    const auto throttle = state.controls.throttle;
    const float targetPitch = 0.8f + (throttle * 0.7f);
    const float targetVolume = 0.2f + (throttle * 0.9f);
    SetMusicPitch(engineSound, targetPitch);
    SetMusicVolume(engineSound, targetVolume);
    UpdateMusicStream(engineSound);

    // update ground height
    const auto x = static_cast<int>(state.position.x / 15.625f);
    const auto z = static_cast<int>(state.position.z / 15.625f);
    if (x < 0 || z < 0 || x >= height->width || z >= height->height) {
        state.groundHeight = 0.0;
    } else {
        const auto r = static_cast<float>(GetImageColor(height, x, z).r);
        state.groundHeight = relativeHeight * r / 255.0f;
    }
    // tiles.update(state.position);
}


void SceneManager::draw() {
    // tiles.draw();
    DrawModel(map, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    // DrawModel(mig, (Vector3){550.0f, 100.0f, 450.0f}, 50.0f, WHITE);
}
