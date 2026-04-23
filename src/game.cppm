module;
#include <entt/entt.hpp>

#include "lib/ray.hpp"
#include "rlgl.h"

export module Game;

import JsonConfig;
import Accessors;
import PlayerSystems;
import AircraftSystems;
import Prefabs;
import WorldStreamerSystem;
import TerrainStreaming;
import RenderSystem;
import ResourceManager;
import Helpers;
import Types;

export class Game {
  entt::registry& registry;
  Scenario scenario{};
  Camera camera = {};
  terrain_streamer::streamer streamer;

 public:
  explicit Game(entt::registry& reg) : registry(reg), scenario(get_scenario(reg)) {
    camera.up = world_up();
    camera.fovy = get_config(registry).player.camera.fov;
    camera.projection = CAMERA_PERSPECTIVE;

    factories::create_player(registry, scenario.start.position);
    // factories::create_scene(registry);
    factories::create_cockpit(registry);
    factories::create_hud(registry);
    factories::create_cockpit_widgets(registry);

    updates::set_minimap(0, registry);
    updates::set_engine_status(1, registry);
    updates::set_radar(2, registry);

    // spawn all items from scenario
    for (const auto& def : scenario.entities) factories::create_unit(registry, def);
  }

  ~Game() {
    registry.ctx().erase<ResourceManager>();
    registry.clear();
  }

  void update() {
    if (is_player_crashed(registry)) return;

    const auto dt = GetFrameTime();
    aircraft_systems::engine(registry, dt);
    aircraft_systems::gear(registry);
    player_systems::controls(registry, dt);
    player_systems::physics(registry, dt);
    player_systems::position(registry, dt);
    player_systems::rotation(registry, dt);
    player_systems::camera(registry, camera);
    player_systems::ground_check(registry, dt);
    aircraft_systems::widgets_inputs(registry);
    aircraft_systems::update_lock(registry);
    streamer.update(registry);
    streamer.process_loaded_chunks(registry);
  }

  void draw() const {
    ClearBackground(scenario.skyColor);

    // 3D
    BeginMode3D(camera);
    // WorldStreamerSystem(registry);
    terrain_streamer::stream(registry);
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
