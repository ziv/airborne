module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"
export module Screens:Game;

import Game;
import JsonConfig;
import Types;
import Accessors;
import :Base;

auto constexpr CONF_PATH = "assets/config.jsonc";
auto constexpr SCENE_PATH = "assets/scenario.jsonc";

export class GameScreen : public BaseScreen {
  entt::registry& registry;
  Game game;

 public:
  explicit GameScreen(entt::registry& reg) : registry(reg), game(reg) {};

  ScreenState update() override {
    game.update();
    if (is_player_crashed(registry) && IsKeyPressed(KEY_SPACE)) return ScreenState::SPLASH;
    return ScreenState::GAMEPLAY;
  }

  void draw() override { game.draw(); }
};
