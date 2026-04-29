#include <entt/entt.hpp>
#include <iostream>
#include <string>

#include "lib/ray-logger.hpp"
#include "lib/ray.hpp"
#include "rlgl.h"

import JsonConfig;
import Accessors;
import Types;
import ResourceManager;
import Resources;
import Screens;

std::unique_ptr<BaseScreen> create_screen(const ScreenState& current, entt::registry& registry) {
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

int main() {
  SetTraceLogCallback(CustomLogCallback);
  SetTraceLogLevel(LOG_DEBUG);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

  try {
    const auto app_conf = JsonConfig(resources::config_path).get<AppConfig>("/config");

    InitWindow(app_conf.global.width, app_conf.global.height, app_conf.global.title.c_str());
    InitAudioDevice();

    // todo remove comment in production
    // SetTargetFPS(60);

    entt::registry registry;
    set_initial_globals(registry, app_conf);

    auto current = ScreenState::SPLASH;
    ScreenState next = current;
    std::unique_ptr<BaseScreen> screen = create_screen(current, registry);

    TraceLog(LOG_DEBUG, "Setting near plane to %f and far plane to %f", app_conf.global.nearPlane, app_conf.global.farPlane);
    rlSetClipPlanes(app_conf.global.nearPlane, app_conf.global.farPlane);

    while (!WindowShouldClose()) {
      if (next = screen->update(); next != current) {
        screen = create_screen(next, registry);
        current = next;
      }
      BeginDrawing();
      screen->draw();
      EndDrawing();
    }

    // make sure we clear everything
    auto& [textures, models, images, shaders, music_streams, sounds] = get_resource_manager(registry);
    models.clear();
    images.clear();
    textures.clear();
    shaders.clear();
    music_streams.clear();
    sounds.clear();

    CloseAudioDevice();
    CloseWindow();
  } catch (std::exception& e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    return -1;
  }
  return 0;
}
