#include "lib/ray-logger.hpp"
#include "rlgl.h"
#include <entt/entt.hpp>
#include <iostream>
#include <raylib.h>
#include <string>

import JsonConfig;
import Types;
// import Game;
// import ResourceManager;
// import ResourcePreloader;
import Screens;

std::unique_ptr<BaseScreen> create_screen(const ScreenState &current,
                                          const JsonConfig &config) {
  switch (current) {
  default:
  case ScreenState::SPLASH:
    return std::make_unique<SplashScreen>(config);

  case ScreenState::GAMEPLAY:
    return std::make_unique<GameScreen>();
  }
}

int main() {
  SetTraceLogCallback(CustomLogCallback);
  SetTraceLogLevel(LOG_DEBUG);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

  try {
    const JsonConfig config("assets/config.jsonc");
    const auto conf = config.get<GlobalConfig>("/global");

    InitWindow(conf.width, conf.height, conf.title.c_str());
    InitAudioDevice();
    // SetTargetFPS(60);

    TraceLog(LOG_DEBUG, "Setting near plane to %f and far plane to %f",
             conf.nearPlane, conf.farPlane);
    rlSetClipPlanes(conf.nearPlane, conf.farPlane);

    auto current = ScreenState::SPLASH;
    std::unique_ptr<BaseScreen> screen = create_screen(current, config);

    while (!WindowShouldClose()) {
      if (const auto next = screen->update(); next != current) {
        screen = create_screen(next, config);
        current = next;
      }
      BeginDrawing();
      screen->run();
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
