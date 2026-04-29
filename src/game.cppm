module;
#include <entt/entt.hpp>

#include "lib/ray.hpp"
#include "rlgl.h"

export module Game;

import JsonConfig;
import Resources;
import Accessors;
import PlayerSystems;
import AircraftSystems;
import Prefabs;
import WorldStreamerSystem;
import TerrainStreaming;
import MapStreaming;
import RenderSystem;
import ResourceManager;
import Helpers;
import Types;
import Components;

export class Game {
  entt::registry& registry;
  // Scenario scenario{};
  Camera camera = {};
  terrain_streamer::streamer streamer;
  map_streamer::streamer map_str;

 public:
  explicit Game(entt::registry& reg) : registry(reg), streamer(reg) {
    camera.up = world_up();
    camera.fovy = get_config(registry).player.camera.fov;
    camera.projection = CAMERA_PERSPECTIVE;

    const auto scene = parse_json_file(resources::scenario_path);

    factories::create_player(registry, scene["data"]["start_position"].get<Vector3>());
    factories::create_scene(registry, scene);
    factories::create_cockpit(registry);
    factories::create_hud(registry);
    factories::create_cockpit_widgets(registry);

    updates::set_minimap(0, registry);
    updates::set_engine_status(1, registry);
    updates::set_radar(2, registry);

    factories::spawn(registry, scene);
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
    map_str.update(registry);
    map_str.process_loaded_tiles(registry);

    // X = zoom out, Z = zoom in on the minimap
    if (const auto view = registry.view<MinimapWidget>(); !view.empty()) {
      auto& wd = registry.get<MinimapWidget>(view.front());
      if (IsKeyPressed(KEY_Z) && wd.map_zoom < 20) wd.map_zoom++;
      if (IsKeyPressed(KEY_X) && wd.map_zoom > 1) wd.map_zoom--;
    }
  }

  void draw() {
    ClearBackground(BLUE);

    // 3D
    BeginMode3D(camera);
    render_systems::sky(registry);
    streamer.stream(registry, camera);
    RenderModels(registry, camera);
    // RenderDebugging(registry);
    EndMode3D();

    // 2D
    // RenderModelsLabel(registry, camera);
    // streamer.draw_tile_labels(registry, camera);
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
