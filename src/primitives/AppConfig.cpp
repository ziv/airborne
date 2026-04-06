#include "AppConfig.h"
#include "Utils.h"

AppConfig::AppConfig() : config(UtilsLoaders::LoadJson("res/config/app.jsonc")) {
}

const nlohmann::json &AppConfig::getNode(const std::string &path) const {
    try {
        return config.at(nlohmann::json::json_pointer(path));
    } catch (const nlohmann::json::exception &e) {
        TraceLog(LOG_ERROR, TextFormat("Config error, missing or invalid path: %s, reason: %s", path.data(), e.what()));
        throw std::runtime_error("Config path not found: " + path);
    }
}
