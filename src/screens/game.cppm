module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module Screens:Game;

import Game;
import JsonConfig;
import Types;
import Accessors;
import :Base;

export class GameScreen : public BaseScreen {
  entt::registry& registry;
  Game game;
  int progress = 0;

 public:
  explicit GameScreen(entt::registry& reg) : registry(reg), game(reg) {}

  ScreenState update() override {
    const int p = game.step();
    if (p >= 0) {
      progress = p;
      return ScreenState::GAMEPLAY;  // stay on this screen, still initialising
    }
    game.update();
    return ScreenState::GAMEPLAY;
  }

  void draw() override {
    if (progress < 100) {
      ClearBackground(BLACK);
      DrawText(TextFormat("Initializing %d%%", progress), 100, 100, 20, GREEN);
      DrawRectangle(100, 150, progress * 4, 30, GREEN);
    } else {
      game.draw();
    }
  }
};
