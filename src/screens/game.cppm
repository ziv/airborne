module;
#include <entt/entt.hpp>

#include "../lib/generator.hpp"
#include "../lib/ray.hpp"

export module Screens:Game;

import Game;
import JsonConfig;
import Resources;
import ResourceManager;
import ResourcePreloader;
import Types;
import Accessors;
import :Base;

// todo finish impl
Generator<float> make_loading_sequence(entt::registry& registry, const nlohmann::json& scene) {
  if (!scene["resources"].is_array()) {
    TraceLog(LOG_DEBUG, "no resources to load, skipping");
    co_return;
  }

  const auto size = static_cast<int>(scene["resources"].size());
  if (0 == size) {
    TraceLog(LOG_DEBUG, "no resources to load, skipping");
    co_return;
  }

  const auto s = static_cast<float>(size);
  auto c = 0.0f;

  for (auto& resource : scene["resources"]) {
    resources::load_resource(registry, resource);
    co_yield ++c / s;
  }
}

export class GameScreen : public BaseScreen {
  entt::registry& registry;
  nlohmann::json scene;
  Game game;
  int loaded = 0;
  int progress = 0;
  bool ready = false;
  bool loading = true;

 public:
  explicit GameScreen(entt::registry& reg)
      : registry(reg),
        // required to load resources and for the game
        scene(parse_json_file(resources::scenario_path)),
        game(registry, scene) {
    // TraceLog(LOG_INFO, "SIZE IN CONSTRUCTOR: %zu", sizeof(GameScreen));
  }

  /// state machine for all game phases
  /// 1. ready - game is playing
  /// 2. loading - loading resources
  /// 3. initializing - spawning all entities
  ScreenState update() override {
    if (ready) {
      game.update();
      return ScreenState::GAMEPLAY;
    }

    // todo move loading resource from resources screen?!
    // todo or just move the generator to resources screen
    // if (loading) {
    //   if (!scene["resources"].is_array()) {
    //     TraceLog(LOG_DEBUG, "no resources to load, skipping");
    //     loading = false;
    //     return ScreenState::GAMEPLAY;
    //   }
    //   const auto size = static_cast<int>(!scene["resources"].size());
    //   if (loaded >= size) {
    //     TraceLog(LOG_DEBUG, "all resources loaded");
    //     loading = false;
    //     return ScreenState::GAMEPLAY;
    //   }
    //   TraceLog(LOG_DEBUG, "preloading resource %d/%d", loaded + 1, size);
    //   resources::load_resource(registry, scene["resources"].at(loaded));
    //   loaded++;
    //   return ScreenState::GAMEPLAY;
    // }

    if (const int p = game.setup(); p >= 0) {
      progress = p;
      return ScreenState::GAMEPLAY;
    }

    ready = true;
    return ScreenState::GAMEPLAY;
  }

  void draw() override {
    if (ready) {
      game.draw();
      return;
    }
    ClearBackground(BLACK);
    DrawText(TextFormat("Initializing %d%%", progress), 100, 100, 20, GREEN);
    DrawRectangle(100, 150, progress * 4, 30, GREEN);
  }
};
