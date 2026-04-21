#include <raylib.h>

#include <entt/entt.hpp>
#include <iostream>
#include <string>

#include "lib/ray-logger.hpp"
#include "rlgl.h"

import JsonConfig;
import Accessors;
import Types;
import Screens;

std::unique_ptr<BaseScreen>
create_screen(const ScreenState& current, entt::registry& registry)
{
  switch (current) {
    default:
    case ScreenState::SPLASH:
      return std::make_unique<SplashScreen>(registry);

    case ScreenState::LOADING:
      return std::make_unique<LoadingScreen>(registry);

    case ScreenState::GAMEPLAY:
      return std::make_unique<GameScreen>(registry);
  }
}

int
main()
{
  SetTraceLogCallback(CustomLogCallback);
  SetTraceLogLevel(LOG_DEBUG);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

  try {
    /// application configuration is split into 2 files:
    /// 1. the aircraft/pilot/game configuration
    /// 2. the scene configuration
    ///    2.1. Entities and objectives
    ///    2.2. Resources list
    const auto json_config = JsonConfig("assets/config.jsonc");
    const auto app_conf = json_config.get<AppConfig>("/config");

    const auto json_scenario = JsonConfig("assets/scenario.jsonc");
    const auto scenario_conf = json_scenario.get<Scenario>("/data");
    const auto resources_conf = json_scenario.get<std::vector<ResourceDef>>("/resources");

    InitWindow(app_conf.global.width, app_conf.global.height, app_conf.global.title.c_str());
    InitAudioDevice();

    // todo remove comment in production
    // SetTargetFPS(60);

    TraceLog(LOG_DEBUG, "Setting near plane to %f and far plane to %f", app_conf.global.nearPlane, app_conf.global.farPlane);
    rlSetClipPlanes(app_conf.global.nearPlane, app_conf.global.farPlane);

    entt::registry registry;
    set_initial_globals(registry, app_conf, scenario_conf, resources_conf);

    auto current = ScreenState::SPLASH;
    ScreenState next = current;
    std::unique_ptr<BaseScreen> screen = create_screen(current, registry);

    while (!WindowShouldClose()) {
      if (next = screen->update(); next != current) {
        screen = create_screen(next, registry);
        current = next;
      }
      BeginDrawing();
      screen->draw();
      EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
  } catch (std::exception& e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    return -1;
  }
  return 0;
}
