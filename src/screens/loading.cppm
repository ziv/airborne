module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module Screens:Loading;

import Game;
import JsonConfig;
import ResourceManager;
import ResourcePreloader;
import Types;
import Accessors;
import :Base;

auto constexpr RESOURCES_PATH = "assets/resources.jsonc";

export class LoadingScreen : public BaseScreen
{
  entt::registry& registry;
  std::vector<ResourceDef> resources{};
  volatile int total{};
  int current{};

public:
  explicit LoadingScreen(entt::registry& r)
    : registry(r)
    , resources(get_resources(r))
    , total(static_cast<int>(resources.size()))
  {
  }

  ScreenState update() override
  {
    // NOLINTBEGIN
    if (total == 0)
      return ScreenState::GAMEPLAY;

    preload_resource(get_resource_manager(registry), resources.at(current));
    current++;
    if (current >= total)
      return ScreenState::GAMEPLAY;

    return ScreenState::LOADING;
    // NOLINTEND
  }

  void draw() override
  {
    ClearBackground(BLACK);
    // NOLINTNEXTLINE
    const int progress = (total == 0) ? 100 : (current * 100) / total;
    DrawText(TextFormat("Loading resources %d%%", progress), 100, 100, 20, GREEN);
    DrawRectangle(100, 150, progress * 4, 30, GREEN);
  }
};
