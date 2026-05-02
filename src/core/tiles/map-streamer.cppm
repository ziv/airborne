module;
#include <algorithm>
#include <entt/entt.hpp>
#include <filesystem>
#include <format>
#include <future>
#include <map>
#include <set>
#include <string>

#include "../../lib/ray.hpp"

export module MapStreaming;

import Components;
import RaylibResource;
import ResourceManager;
import Accessors;
import Types;
import TileDownloader;

/// map streamer is a way simpler version of the terrain streamer:
/// 1. 2D only
/// 2. Fixed 5x5 tile grid centred on the player
/// 3. Map tiles downloaded on demand from TomTom (road style)
/// 4. Supports all zoom levels 1–20 (zoom < 12 uses tiles larger than TILE_SIZE_12)

constexpr int MAP_GRID_HALF = 1;          // tiles in each direction → 5×5 grid
constexpr int MAP_TILE_PX = 256;          // each downloaded tile is 256×256 px
constexpr float TILE_SIZE_Z12 = 9783.9f;  // world metres per z12 tile (matches terrain streamer)
constexpr int MAP_BASE_X = 2444;          // geographic TMS x of local-origin tile at zoom 12
constexpr int MAP_BASE_Z = 1655;          // geographic TMS z of local-origin tile at zoom 12
constexpr int MAP_ANCHOR_ZOOM = 12;

// World metres per tile at any zoom level.
float map_tile_size(const int zoom) {
  const int diff = zoom - MAP_ANCHOR_ZOOM;
  if (diff >= 0) return TILE_SIZE_Z12 / static_cast<float>(1 << diff);
  return TILE_SIZE_Z12 * static_cast<float>(1 << -diff);
}

// Compute the geographic (TMS) tile index directly from a world coordinate.
// Avoids all intermediate integer truncation by doing one float floor at the end.
// geo = floor((world + BASE * TILE_Z12) / tile_size(zoom))
int world_to_geo(const float world, const int base, const int zoom) {
  const float origin = static_cast<float>(base) * TILE_SIZE_Z12;
  return static_cast<int>(std::floor((world + origin) / map_tile_size(zoom)));
}

// Compute the local tile index for a world coordinate at the given zoom.
int world_to_local(const float world, const int zoom) { return static_cast<int>(std::floor(world / map_tile_size(zoom))); }

int map_tile_id(const int zoom, const int x, const int z) { return static_cast<int>(entt::hashed_string(TextFormat("map_tile_%d_%d_%d", zoom, x, z)).value()); }

Image load_map_tile(const int zoom, const int geo_tx, const int geo_tz) {
  const std::string path = std::format("assets/tiles/map/{}/{}/{}.png", zoom, geo_tx, geo_tz);
  const auto tomtom_token = std::string(std::getenv("TOMTOM_TOKEN"));
  const std::string url = tile_downloader::map_url(zoom, geo_tx, geo_tz, tomtom_token);
  tile_downloader::enqueue(path, url).wait();
  return LoadImage(path.c_str());
}

// ---------------------------------------------------------------------------
// Components (internal — not exported as ECS components, stored on entities
// alongside the map tile data)
// ---------------------------------------------------------------------------

export struct AsyncMapTileLoad {
  std::future<Image> future;
  int zoom = 14;
  int x = 0;  // local index
  int z = 0;
  int geo_x = 0;  // geographic TMS index
  int geo_z = 0;
};

export struct MapTile {
  int tex_id = 0;  // key into rm.textures
  int zoom = 14;
  int x = 0;  // local index
  int z = 0;
  int geo_x = 0;  // geographic TMS index (used for rendering position)
  int geo_z = 0;
};

// ---------------------------------------------------------------------------
// Streamer
// ---------------------------------------------------------------------------

struct TileKey {
  int zoom, x, z;            // local indices (used for tracking + rendering)
  int geo_x = 0, geo_z = 0;  // geographic TMS indices (used for download)
  // Comparison only on logical identity — zoom+local coords.
  bool operator==(const TileKey& o) const { return zoom == o.zoom && x == o.x && z == o.z; }
  bool operator<(const TileKey& o) const {
    if (zoom != o.zoom) return zoom < o.zoom;
    if (x != o.x) return x < o.x;
    return z < o.z;
  }
};

export namespace map_streamer {

class streamer {
  std::map<TileKey, entt::entity> tracked_tiles;
  std::string token;

 public:
  explicit streamer(entt::registry& registry) : token(get_options(registry).maps_token) {}
  void update(entt::registry& registry) {
    const auto& player = get_player(registry);
    const auto pos = player.absolute_position();

    int map_zoom = 12;
    if (const auto view = registry.view<MinimapWidget>(); !view.empty()) {
      map_zoom = registry.get<MinimapWidget>(view.front()).map_zoom;
    }

    const int cx = world_to_local(pos.x, map_zoom);
    const int cz = world_to_local(pos.z, map_zoom);

    // geo tile of the player's exact position — computed once in float to avoid
    // integer truncation errors that grow at coarser zoom levels.
    const int geo_cx = world_to_geo(pos.x, MAP_BASE_X, map_zoom);
    const int geo_cz = world_to_geo(pos.z, MAP_BASE_Z, map_zoom);

    std::set<TileKey> desired;
    for (int dx = -MAP_GRID_HALF; dx <= MAP_GRID_HALF; ++dx)
      for (int dz = -MAP_GRID_HALF; dz <= MAP_GRID_HALF; ++dz) desired.insert({map_zoom, cx + dx, cz + dz, geo_cx + dx, geo_cz + dz});

    // Spawn newly desired tiles.
    for (const auto& key : desired) {
      if (!tracked_tiles.contains(key)) tracked_tiles[key] = spawn_tile(registry, key);
    }

    // evict tiles no longer desired.
    std::erase_if(tracked_tiles, [&](const auto& item) {
      const auto [key, entity] = item;
      if (desired.contains(key)) return false;
      get_resource_manager(registry).textures.erase(map_tile_id(key.zoom, key.x, key.z));
      registry.destroy(entity);
      return true;
    });
  }

  void process_loaded_tiles(entt::registry& registry) {
    for (const auto view = registry.view<AsyncMapTileLoad>(); const auto [entity, tile] : view.each()) {
      if (tile.future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) continue;

      const Image img = tile.future.get();
      const Texture2D tex = LoadTextureFromImage(img);
      UnloadImage(img);

      const int zoom = tile.zoom;
      const int tx = tile.x;
      const int tz = tile.z;
      const int geo_x = tile.geo_x;
      const int geo_z = tile.geo_z;
      const int tid = map_tile_id(zoom, tx, tz);

      get_resource_manager(registry).textures.load(tid, tex);

      registry.emplace<MapTile>(entity, tid, zoom, tx, tz, geo_x, geo_z);
      registry.remove<AsyncMapTileLoad>(entity);
      break;
    }
  }

  [[nodiscard]] static int get_tile_tex(entt::registry& registry, const int zoom, const int x, const int z) {
    const int tid = map_tile_id(zoom, x, z);
    return get_resource_manager(registry).textures.contains(tid) ? tid : -1;
  }

 private:
  static entt::entity spawn_tile(entt::registry& registry, const TileKey& key) {
    const auto entity = registry.create();
    auto task = std::async(std::launch::async, [zoom = key.zoom, geo_tx = key.geo_x, geo_tz = key.geo_z]() { return load_map_tile(zoom, geo_tx, geo_tz); });
    registry.emplace<AsyncMapTileLoad>(entity, std::move(task), key.zoom, key.x, key.z, key.geo_x, key.geo_z);
    return entity;
  }
};

}  // namespace map_streamer
