#include <entt/entt.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

#include "lib/ray-logger.hpp"
#include "lib/ray.hpp"
#include "rlgl.h"

import JsonConfig;
import AppOptions;
import Accessors;
import Types;
import ResourceManager;
import Resources;
import Screens;
import TileDownloader;

/// screen selectors, create the required screen object
/// screen are unique_ptr by design
std::unique_ptr<BaseScreen> create_screen(const ScreenState& current, entt::registry& registry) {
  TraceLog(LOG_INFO, "SIZE IN FACTORY: %zu", sizeof(GameScreen));
  switch (current) {
    default:
    case ScreenState::SPLASH:
      return std::make_unique<SplashScreen>(registry);

    case ScreenState::GAMEPLAY:
      return std::make_unique<GameScreen>(registry);
  }
}

/// load configuration files and ensure existing of required tokens in AppOptions.
/// tokens can be set in the options file or in environment variables.
/// @see https://github.com/ziv/airborne/wiki/Tokens for more information
std::tuple<AppConfig, AppOptions> load_requirements() {
  // while config is a plain object, app options contain method
  // to save data that changed during the game
  const auto app_conf = get_json_node<AppConfig>(parse_json_file(resources::config_path), "/config");
  const AppOptions options{resources::options_path};
  return {app_conf, options};
}

int main() {
  // setup raylib
  SetTraceLogCallback(CustomLogCallback);
  SetTraceLogLevel(LOG_DEBUG);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

  try {
    const auto [config, options] = load_requirements();

    // init devices
    InitWindow(config.global.width, config.global.height, config.global.title.c_str());
    InitAudioDevice();
    rlSetClipPlanes(config.global.nearPlane, config.global.farPlane);

    // todo remove comment in production
    // SetTargetFPS(60);

    // game registry
    entt::registry registry = initial_registry(config, options);

    // screens state
    auto current = ScreenState::SPLASH;
    auto next = ScreenState::SPLASH;
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

    // make sure we clear everything
    screen = nullptr;
    resources::unload_resource_manager(registry);
    registry.clear();

    // Tear down the tile downloader pool while libssl is still alive. Doing
    // this here (instead of relying on static destruction) avoids a crash in
    // ssl3_shutdown caused by OpenSSL globals being finalized first.
    tile_downloader::shutdown();

    CloseAudioDevice();
    CloseWindow();
  } catch (std::exception& e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    return -1;
  }
  return 0;
}
