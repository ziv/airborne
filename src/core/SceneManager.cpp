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
}

SceneManager::~SceneManager() {
    UnloadImage(height);
}

// void SceneManager::addChunk(Model rawModel, Vector3 position) {
//     chunks.emplace_back(rawModel, position);
// }

// void SceneManager::loadChunk(int x, int z) {
//     const std::string txPath = "res/16kparts/tx-" + std::to_string(x) + "-" + std::to_string(z) + ".png";
//     const std::string hmPath = "res/16kparts/hm-" + std::to_string(x) + "-" + std::to_string(z) + ".png";
//     auto model = UtilsLoaders::loadTerrain(txPath, hmPath, {2048, 8192, 2048});
//
//     activeChunks.emplace(std::piecewise_construct,
//                              std::forward_as_tuple(x, z),
//                              std::forward_as_tuple(model));
// }


void SceneManager::update(AircraftState &state, float dt) {
    const auto x = static_cast<int>(state.position.x / 62.5f);
    const auto z = static_cast<int>(state.position.z / 62.5f);
    if (x < 0 || z < 0 || x > height.width || z > height.height) {
        state.groundHeight = 0.0;
    } else {
        state.groundHeight = static_cast<float>(3200.0 * GetImageColor(height, x, z).r / 255.0);
    }

    // float cameraPos[3] = { state.position.x, state.position.y, state.position.z };
    // SetShaderValue(fogShader, camPosLoc, cameraPos, SHADER_UNIFORM_VEC3);
    // static_cast<Model>(map).materials[0].shader = fogShader;

    // const int currentX = static_cast<int>(floor(state.position.x / CHUNK_SIZE));
    // const int currentZ = static_cast<int>(floor(state.position.z / CHUNK_SIZE));
    // // build the list of required vectors
    // std::vector<std::pair<int, int> > requiredCoords;
    // for (int x = currentX - 1; x <= currentX + 1; ++x) {
    //     for (int z = currentZ - 1; z <= currentZ + 1; ++z) {
    //         if (x >= 0 && x < GRID_LIMIT && z >= 0 && z < GRID_LIMIT) {
    //             requiredCoords.emplace_back(x, z);
    //         }
    //     }
    // }
    // // clean what we do not need anymore
    // for (auto it = activeChunks.begin(); it != activeChunks.end();) {
    //     bool isStillNeeded = false;
    //     for (const auto &req: requiredCoords) {
    //         if (it->first == req) {
    //             isStillNeeded = true;
    //             break;
    //         }
    //     }
    //
    //     if (!isStillNeeded) {
    //         it = activeChunks.erase(it);
    //     } else {
    //         ++it;
    //     }
    // }
    // // load what needed
    // for (const auto& req : requiredCoords) {
    //     if (activeChunks.find(req) == activeChunks.end()) {
    //         loadChunk(req.first, req.second);
    //     }
    // }
}

void SceneManager::draw() const {
    DrawModel(map, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    // for (auto const& [coords, model] : activeChunks) {
    //     Vector3 pos = { coords.first * CHUNK_SIZE, 0, coords.second * CHUNK_SIZE };
    //     DrawModel(model, pos, 1.0f, WHITE);
    // }
    //
    // DrawText(TextFormat("Active Chunks: %d", (int)activeChunks.size()), 10, 10, 20, RED);
}

// float SceneManager::getHeight(const int x, const int z) const {
//     return relativeHeight * static_cast<float>(GetImageColor(height, x, z).r) / 255.0f;
// }
