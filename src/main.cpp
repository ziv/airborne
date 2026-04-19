#include "lib/ray-logger.hpp"
#include "rlgl.h"
#include <entt/entt.hpp>
#include <iostream>
#include <raylib.h>
#include <string>

import JsonConfig;
import Types;
import Screens;

std::unique_ptr<BaseScreen> create_screen(const ScreenState &current,
                                          entt::registry &registry) {
  switch (current) {
  default:
  case ScreenState::SPLASH:
    return std::make_unique<SplashScreen>();

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
    const auto conf =
        JsonConfig("assets/main.jsonc").get<GlobalConfig>("/global");

    InitWindow(conf.width, conf.height, conf.title.c_str());
    InitAudioDevice();
    // SetTargetFPS(60);

    TraceLog(LOG_DEBUG, "Setting near plane to %f and far plane to %f",
             conf.nearPlane, conf.farPlane);
    rlSetClipPlanes(conf.nearPlane, conf.farPlane);

    entt::registry registry;
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
  } catch (std::exception &e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    return -1;
  }
  return 0;
}
