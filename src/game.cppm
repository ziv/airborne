module;
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <optional>

#include "lib/generator.hpp"
#include "lib/ray.hpp"

export module Game;

import JsonConfig;
import Resources;
import Accessors;
import PlayerSystems;
import AircraftSystems;
import Npc;
import Prefabs;
import TerrainStreaming;
import MapStreaming;
import RenderSystem;
import ResourceManager;
import Helpers;
import Types;
import Components;
import GameOptions;

Generator<int> make_setup_sequence(entt::registry& registry, const nlohmann::json& scene) {
  factories::create_player(registry, scene);
  co_yield 10;
  factories::create_scene(registry, scene);
  co_yield 20;
  factories::create_engine(registry);
  co_yield 30;
  factories::create_cockpit(registry);
  co_yield 40;
  factories::create_hud(registry);
  co_yield 50;
  factories::create_cockpit_widgets(registry);
  co_yield 55;

  updates::set_minimap(0, registry);
  // updates::set_engine_status(1, registry);
  updates::set_target_camera(1, registry);
  updates::set_radar(2, registry);
  co_yield 65;

  const auto& entities = scene["entities"];
  const int n = static_cast<int>(entities.size());
  for (int i = 0; i < n; ++i) {
    factories::spawn_one(registry, entities[i]);
    co_yield 65 + 30 * (i + 1) / std::max(n, 1);  // 65 → 95
  }

  npc_systems::setup(registry);
  registry.ctx().get<GameState>().status = GameStatus::PLAYING;
  co_yield 100;
}

export class Game {
  entt::registry& registry;
  Camera camera = {};
  nlohmann::json scene;
  std::optional<Generator<int>> setup_gen;
  terrain_streamer::streamer streamer;
  map_streamer::streamer map_str;

 public:
  explicit Game(entt::registry& reg) : registry(reg), scene(parse_json_file(resources::scenario_path)), streamer(reg) {
    camera.up = world_up();
    camera.projection = CAMERA_PERSPECTIVE;
    setup_gen.emplace(make_setup_sequence(registry, scene));
  }

  ~Game() {
    registry.ctx().erase<ResourceManager>();
    registry.clear();
  }

  // advance one setup step. Returns progress [0,100], or -1 when done.
  int setup() {
    if (setup_gen->next()) return setup_gen->current();
    setup_gen.reset();
    return -1;
  }

  void update() {
    if (is_player_crashed(registry)) return;

    if (IsKeyPressed(KEY_P)) {
      if (auto& [status] = registry.ctx().get<GameState>(); status == GameStatus::PLAYING)
        status = GameStatus::PAUSED;
      else if (status == GameStatus::PAUSED)
        status = GameStatus::PLAYING;
    }

    if (const auto [status] = registry.ctx().get<GameState>(); status == GameStatus::PAUSED) {
      player_systems::camera(registry, camera);
      return;
    }

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
    npc_systems::autopilot(registry, dt);
    npc_systems::physics(registry, dt);
    streamer.update(registry);
    streamer.process_loaded_chunks(registry);
    map_str.update(registry);
    map_str.process_loaded_tiles(registry);

    if (const auto view = registry.view<MinimapWidget>(); !view.empty()) {
      auto& wd = registry.get<MinimapWidget>(view.front());
      if (IsKeyPressed(KEY_Z) && wd.map_zoom < 20) wd.map_zoom++;
      if (IsKeyPressed(KEY_X) && wd.map_zoom > 1) wd.map_zoom--;
    }
  }

  void draw() const {
    ClearBackground(BLUE);

    BeginMode3D(camera);
    render_systems::sky(registry);
    // streamer.stream_debug(registry, camera);
    streamer.stream(registry, camera);
    render_systems::models(registry);
    render_systems::debug_models(registry);
    EndMode3D();

    terrain_streamer::streamer::draw_tile_labels(registry, camera);
    RenderCockpit(registry);
    RenderMinimap(registry);
    RenderEngineStatus(registry);
    RenderHud(registry);
    RenderRadar(registry);
    render_systems::target_camera(registry);
    DrawFPS(1050, 780);
    RenderDebug(registry);
    RenderCrashLayout(registry);

    game_options::options(registry);
  }
};
