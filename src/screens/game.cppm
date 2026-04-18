module;
#include "../lib/ray.hpp"
#include <entt/entt.hpp>
export module Screens:Game;

import Game;
import JsonConfig;
import :Base;

export class GameScreen : public BaseScreen {
  entt::registry registry;
  Game game;

public:
  explicit GameScreen()
      : game(JsonConfig("assets/config.jsonc"),
             JsonConfig("assets/scenario.jsonc"), registry) {}

  ScreenState update() override {
    game.update();
    return ScreenState::GAMEPLAY;
  }

  void draw() override { game.draw(); }
};