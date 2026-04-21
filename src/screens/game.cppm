module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"
export module Screens:Game;

import Game;
import JsonConfig;
import Types;
import :Base;

auto constexpr CONF_PATH = "assets/config.jsonc";
auto constexpr SCENE_PATH = "assets/scenario.jsonc";

export class GameScreen : public BaseScreen
{
  entt::registry& registry;
  Game game;

public:
  explicit GameScreen(entt::registry& reg)
    : registry(reg)
    , game(reg) {};

  ScreenState update() override
  {
    game.update();
    return ScreenState::GAMEPLAY;
  }

  void draw() override { game.draw(); }
};
