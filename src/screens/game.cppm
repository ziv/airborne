module;
#include <entt/entt.hpp>

export module Screens:Game;

import Game;
import JsonConfig;
import Types;
import Accessors;
import :Base;

export class GameScreen : public BaseScreen {
  entt::registry& registry;
  Game game;

 public:
  explicit GameScreen(entt::registry& reg) : registry(reg), game(reg) {};

  ScreenState update() override {
    game.update();
    return ScreenState::GAMEPLAY;
  }

  void draw() override { game.draw(); }
};
