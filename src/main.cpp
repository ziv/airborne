#include <entt/entt.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
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

/// load configuration files and ensure existing of required tokens.
/// tokens can be set in the options file or in environment variables.
/// @see https://github.com/ziv/airborne/wiki/Tokens for more information
std::tuple<AppConfig, nlohmann::json> load_requirements() {
  const auto app_conf = JsonConfig(resources::config_path).get<AppConfig>("/config");
  auto options = parse_json_file(resources::options_path);

  if (!options.contains("tiles_token")) {
    const auto tiles_token = std::string(std::getenv(resources::tiles_token_name));
    if (tiles_token.empty()) throw std::runtime_error("missing tiles token in options or environment variables");
    options["tiles_token"] = tiles_token;
  }

  if (!options.contains("maps_token")) {
    const auto maps_token = std::string(std::getenv(resources::maps_token_name));
    if (maps_token.empty()) throw std::runtime_error("missing maps token in options or environment variables");
    options["maps_token"] = maps_token;
  }

  return {app_conf, options};
}

int main() {
  SetTraceLogCallback(CustomLogCallback);
  SetTraceLogLevel(LOG_DEBUG);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

  try {
    // const auto app_conf = JsonConfig(resources::config_path).get<AppConfig>("/config");
    // const auto options = parse_json_file(resources::options_path);

    const auto [app_conf, options] = load_requirements();
    InitWindow(app_conf.global.width, app_conf.global.height, app_conf.global.title.c_str());
    InitAudioDevice();
    rlSetClipPlanes(app_conf.global.nearPlane, app_conf.global.farPlane);

    // todo remove comment in production
    // SetTargetFPS(60);

    entt::registry registry;
    set_initial_globals(registry, app_conf, options);

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

    // make sure we clear everything
    unload_resource_manager(registry);

    CloseAudioDevice();
    CloseWindow();
  } catch (std::exception& e) {
    std::cerr << "Fatal Error: " << e.what() << std::endl;
    return -1;
  }
  return 0;
}
