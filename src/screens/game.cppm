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
  Game game{JsonConfig(CONF_PATH),JsonConfig(SCENE_PATH),registry};

public:
  explicit GameScreen(entt::registry &r) : registry(r){};

  ScreenState update() override {
    game.update();
    return ScreenState::GAMEPLAY;
  }

  void draw() override { game.draw(); }
};
