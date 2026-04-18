#include <entt/entt.hpp>
#include <raylib.h>
#include <iostream>
#include <string>
#include "lib/ray-logger.hpp"
#include "rlgl.h"

import JsonConfig;
import Types;
import Game;
import ResourceManager;
import ResourcePreloader;

int main() {
    SetTraceLogCallback(CustomLogCallback);
    SetTraceLogLevel(LOG_DEBUG);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

    try {
        const JsonConfig config("assets/config.jsonc");
        const JsonConfig scenario_def("assets/scenario.jsonc");

        const auto conf = config.get<GlobalConfig>("/global");

        InitWindow(conf.width, conf.height, conf.title.c_str());
        InitAudioDevice();
        // SetTargetFPS(60);

        TraceLog(LOG_DEBUG, "Setting near plane to %f and far plane to %f", conf.nearPlane, conf.farPlane);
        rlSetClipPlanes(conf.nearPlane, conf.farPlane);

        // set registry & resource loader & global config
        entt::registry registry;
        create_resource_manager(registry);
        registry.ctx().emplace<GlobalConfig>(conf);

        // preload all required resources
        preload_resources(registry);

        Game game(config, scenario_def, registry);

        while (!WindowShouldClose()) {
            game.update();
            BeginDrawing();
            game.draw();
            EndDrawing();
        }

        registry.ctx().erase<ResourceManager>();
        registry.clear();

        CloseAudioDevice();
        CloseWindow();
    } catch (std::exception &e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
