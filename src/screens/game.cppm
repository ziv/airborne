module;
#include "../lib/ray.hpp"
#include <entt/entt.hpp>
export module Screens:Game;

import Game;
import JsonConfig;
import :Base;

export class GameScreen : public BaseScreen {
  JsonConfig config;
  JsonConfig scenario;

  entt::registry registry;
  Game game;

public:
  explicit GameScreen()
      : config(JsonConfig("assets/config.jsonc")),
        scenario(JsonConfig("assets/scenario.jsonc")),
        game(config, scenario, registry) {}

  ScreenState update() override {
    game.update();
    return ScreenState::GAMEPLAY;
  }

  void run() override { game.draw(); }
};