#include "SceneManager.h"
#include "../primitives/Utils.h"

SceneManager::SceneManager(const AppConfig &config) : engineSound(LoadMusicStream(config.get<std::string_view>("/game/engineSound").data())),
                                                      height(LoadImage(config.get<std::string_view>("/game/mapHeightmap").data())),
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
}

void SceneManager::update(const AircraftState &state, float dt) {
}

void SceneManager::draw(float dt) {
}

float SceneManager::getHeight(const int x, const int z) const {
    return relativeHeight * static_cast<float>(GetImageColor(height, x, z).r) / 255.0f;
}
