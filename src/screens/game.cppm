module;
#include "../lib/ray.hpp"
#include <entt/entt.hpp>
export module Screens:Game;

import Game;
import JsonConfig;
import :Base;

auto constexpr CONF_PATH = "assets/config.jsonc";
auto constexpr SCENE_PATH = "assets/scenario.jsonc";

export class GameScreen : public BaseScreen {
  entt::registry &registry;
  Game game;

public:
  explicit GameScreen(entt::registry &r, const JsonConfig &conf,
                      const JsonConfig &scene)
      : registry(r), game(conf, scene, registry) {};

  ScreenState update() override {
    game.update();
    return ScreenState::GAMEPLAY;
  }

  void draw() override { game.draw(); }
};
