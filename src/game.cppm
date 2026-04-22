module;
#include <entt/entt.hpp>

#include "lib/ray.hpp"

export module Game;

import JsonConfig;
import PlayerSystems;
import Player;
import Components;
import Prefabs;
import WorldStreamerSystem;
import RenderSystem;
import Types;
import ResourceManager;
import WidgetsInputs;
import Accessors;
import ResourcePreloader;
import EngineSoundSystem;
import UpdateLockSystem;
import GearSoundSystem;
import TerrainStreaming;
import Accessors;

export class Game
{
  entt::registry& registry;
  Scenario scenario{};
  PlayerDispatcher dispatcher;

public:
  explicit Game(entt::registry& reg)
    : registry(reg)
    , scenario(get_scenario(reg))
    , dispatcher(reg)
  {
    factories::create_player(registry, scenario.start.position);
    factories::create_scene(registry);
    factories::create_cockpit(registry);
    factories::create_hud(registry);
    factories::create_cockpit_widgets(registry);

    updates::set_minimap(0, registry);
    updates::set_engine_status(1, registry);
    updates::set_radar(2, registry);

    // spawn all items from scenario
    for (const auto& def : scenario.entities)
      factories::create_unit(registry, def);
  }

  ~Game()
  {
    registry.ctx().erase<ResourceManager>();
    registry.clear();
  }

  void update()
  {
    if (is_player_crashed(registry))
      return;

    const auto dt = GetFrameTime();
    // inputs
    EngineSystem(registry, dt);
    GearSystem(registry);
    player_systems::controls(registry, dt);
    player_systems::physics(registry, dt);
    player_systems::position(registry, dt);
    player_systems::rotation(registry, dt);
    dispatcher.update(registry, dt);
    player_systems::ground_check(registry, dt);
    // the rest
    WidgetsInputs(registry);
    UpdateLockSystem(registry);
  }

  void draw()
  {
    ClearBackground(scenario.skyColor);

    // 3D
    BeginMode3D(dispatcher.playerCamera.getCamera());
    WorldStreamerSystem(registry);
    RenderModels(registry);
    RenderDebugging(registry);
    EndMode3D();

    // 2D
    RenderCockpit(registry);
    RenderMinimap(registry);
    RenderEngineStatus(registry);
    RenderHud(registry);
    RenderRadar(registry);
    DrawFPS(1050, 780);
    RenderDebug(registry);
    RenderCrashLayout(registry);
  }
};
