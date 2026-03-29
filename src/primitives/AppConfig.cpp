#include "AppConfig.h"
#include <fstream>
#include "raylib.h"

AppConfig::AppConfig() {
    std::ifstream file("app.json");
    if (!file.is_open()) {
        throw std::runtime_error("Could not open app.json");
    }
    file >> config;
    file.close();
    TraceLog(LOG_INFO, "[AppConfig] configuration loaded");
}