/**
 * @file ScenarioLoader.cpp
 * @brief JSON deserialization for scenarios, entities, objectives, and triggers.
 */
#include "ScenarioLoader.h"
#include "../primitives/Utils.h"
#include <filesystem>

using json = nlohmann::json;

Scenario ScenarioLoader::load(const std::string &path) {
    TraceLog(LOG_INFO, "Loading scenario: %s", path.c_str());
    const json data = UtilsLoaders::LoadJson(path);
    return data.get<Scenario>();
}

std::vector<Scenario> ScenarioLoader::loadAll(const std::string &configPath) {
    TraceLog(LOG_INFO, "Loading all scenarios from: %s", configPath.c_str());
    const json data = UtilsLoaders::LoadJson(configPath);

    std::vector<Scenario> scenarios;
    if (data.contains("scenarios")) {
        for (const auto &entry: data["scenarios"]) {
            try {
                scenarios.push_back(entry.get<Scenario>());
            } catch (const std::exception &e) {
                TraceLog(LOG_WARNING, "Failed to parse scenario: %s", e.what());
            }
        }
    }
    TraceLog(LOG_INFO, "Loaded %zu scenarios", scenarios.size());
    return scenarios;
}

std::vector<std::string> ScenarioLoader::listAvailable(const std::string &scenariosDir) {
    std::vector<std::string> files;
    try {
        for (const auto &entry: std::filesystem::directory_iterator(scenariosDir)) {
            if (entry.path().extension() == ".jsonc") {
                files.push_back(entry.path().string());
            }
        }
        std::sort(files.begin(), files.end());
    } catch (const std::filesystem::filesystem_error &e) {
        TraceLog(LOG_WARNING, "Could not list scenarios in %s: %s", scenariosDir.c_str(), e.what());
    }
    return files;
}
