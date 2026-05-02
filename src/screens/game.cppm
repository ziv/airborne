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

Generator<int> make_loading_sequence(entt::registry& registry, const nlohmann::json& scene) {
  TraceLog(LOG_DEBUG, "start loading resources");
  if (!scene["resources"].is_array()) {
    TraceLog(LOG_DEBUG, "no resources to load, skipping");
    co_return;
  }

  const auto size = static_cast<int>(scene["resources"].size());
  if (0 == size) {
    TraceLog(LOG_DEBUG, "no resources to load, skipping");
    co_return;
  }
  TraceLog(LOG_DEBUG, "loading %d resources", size);

  const auto s = static_cast<float>(size);
  float c = 0;

  for (auto& resource : scene["resources"]) {
    TraceLog(LOG_DEBUG, "loading resource %d/%d: %s", c + 1, size, resource.dump().c_str());
    resources::load_resource(registry, resource);
    co_yield static_cast<int>(++c / s * 100.0f);
  }
}

export class GameScreen : public BaseScreen {
  entt::registry& registry;
  nlohmann::json scene;
  Game game;
  Generator<int> loading_gen;
  int loaded = 0;
  int progress = 0;
  bool ready = false;
  bool loading = true;

 public:
  explicit GameScreen(entt::registry& reg)
      : registry(reg),
        // required to load resources and for the game
        scene(parse_json_file(resources::scenario_path)),
        game(registry, scene),
        loading_gen(make_loading_sequence(registry, scene)) {
    TraceLog(LOG_INFO, "SIZE IN CONSTRUCTOR: %zu", sizeof(GameScreen));
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

    if (loading) {
      if (loading_gen.next()) {
        loaded = loading_gen.current();
        return ScreenState::GAMEPLAY;  // progress is not used for now, but can be used to show loading progress
      }
      loading = false;
    }

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
    if (loading) {
      DrawText(TextFormat("Loading resources %d%%", loaded), 100, 100, 20, GREEN);
      DrawRectangle(100, 150, loaded * 4, 30, GREEN);
    } else {
      DrawText(TextFormat("Initializing %d%%", progress), 100, 100, 20, GREEN);
      DrawRectangle(100, 150, progress * 4, 30, GREEN);
    }
  }
};
