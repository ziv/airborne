module;
#include "../lib/ray.hpp"
#include <entt/entt.hpp>

export module Screens:Loading;

import Game;
import JsonConfig;
import ResourceManager;
import ResourcePreloader;
import :Base;

auto constexpr RESOURCES_PATH = "assets/resources.jsonc";

export class LoadingScreen : public BaseScreen {
  entt::registry &registry;
  std::vector<ResourceDef> resources;
  int total = 0;
  int current = 0;

public:
  explicit LoadingScreen(entt::registry &r)
      : registry(r),
        resources(JsonConfig(RESOURCES_PATH)
                      .get<std::vector<ResourceDef>>("/resources")),
        total(static_cast<int>(resources.size())) {
    create_resource_manager(registry);
  }

  ScreenState update() override {
    if (total == 0)
      return ScreenState::GAMEPLAY;

    preload_resource(get_resource_manager(registry), resources.at(current));
    current++;
    if (current >= total)
      return ScreenState::GAMEPLAY;

    return ScreenState::LOADING;
  }

  void draw() override {
    ClearBackground(BLACK);
    const int progress = (total == 0) ? 100 : (current * 100) / total;
    DrawText(TextFormat("Loading resources %d%%", progress), 100, 100, 20,
             GREEN);
    DrawRectangle(100, 150, progress * 4, 30, GREEN);
  }
};
