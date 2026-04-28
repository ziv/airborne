module;
#include <entt/entt.hpp>
#include <future>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "../../lib/ray.hpp"

export module TerrainStreaming;

import Components;
import RaylibResource;
import ResourceManager;
import Resources;
import Accessors;
import Types;

// LOD zoom levels — ordered highest (closest) to lowest (farthest)
struct ZoomLevel {
  int zoom;
  Meter tile_size;
  int base_x;
  int base_z;
  int radius;  // half-extent: radius 3 → 7×7 grid
};

constexpr ZoomLevel LOD_LEVELS[] = {
    {14, 2445.975f, 9755, 6627, 3},
    {13, 4891.95f, 4870, 3306, 3},
    {12, 9783.9f, 2435, 1653, 3},
};
constexpr int LOD_COUNT = 3;

Model create_lod_model(const Meter tile_size) { return LoadModelFromMesh(GenMeshPlane(tile_size, tile_size, 256, 256)); }

export struct AsyncTileLoad {
  std::future<Image> texture_future;
  std::future<Image> heightmap_future;
  int zoom = 0;
  int x = 0;
  int z = 0;
};

export struct TerrainChunk {
  int model;
  int height;
  int zoom;
};

export struct TerrainHeight {
  int height;  // id of the height model
};

// tag: this tile is being replaced by higher-zoom children — keep rendering until they load
export struct PendingReplacement {
  std::vector<entt::entity> replacements;
};

struct TileKey {
  int zoom = 0;
  int x = 0;
  int z = 0;

  auto operator<=>(const TileKey&) const = default;
};

inline int get_tex_id(const int zoom, const int x, const int z) {
  return entt::hashed_string(TextFormat("tile_tex_%d_%d_%d", zoom, x, z));
}
inline int get_height_id(const int zoom, const int x, const int z) {
  return entt::hashed_string(TextFormat("tile_height_%d_%d_%d", zoom, x, z));
}

// check if a tile at `parent_zoom` coords (px, pz) is fully covered by higher-zoom tiles in `required`
inline bool is_covered(const int parent_zoom, const int px, const int pz, const std::set<TileKey>& required) {
  const int child_zoom = parent_zoom + 1;
  const int cx = px * 2;
  const int cz = pz * 2;
  return required.contains({child_zoom, cx, cz}) && required.contains({child_zoom, cx + 1, cz}) &&
         required.contains({child_zoom, cx, cz + 1}) && required.contains({child_zoom, cx + 1, cz + 1});
}

export namespace terrain_streamer {

void on_terrain_destroyed(entt::registry& reg, entt::entity entity) {
  TraceLog(LOG_INFO, "Destroying terrain chunk entity %d", static_cast<int>(entity));
}

class streamer {
  std::map<TileKey, entt::entity> active_tiles;
  // per-zoom "last player tile" — used to detect movement
  int last_tile_x[LOD_COUNT] = {-9999, -9999, -9999};
  int last_tile_z[LOD_COUNT] = {-9999, -9999, -9999};
  Shader displacement_shader;
  std::map<int, Model> terrain_models;  // zoom → Model

  void setup_model_shader(Model& model) const {
    model.materials[0].shader = displacement_shader;
  }

 public:
  explicit streamer(entt::registry& registry)
      : displacement_shader(LoadShader("assets/shaders/terrain.vs", "assets/shaders/terrain.fs")) {
    // create one model per LOD zoom level
    for (int i = 0; i < LOD_COUNT; ++i) {
      auto model = create_lod_model(LOD_LEVELS[i].tile_size);
      setup_model_shader(model);
      terrain_models[LOD_LEVELS[i].zoom] = model;
    }

    // heightmap sampler slot
    constexpr int heightmapSlotIndex = MATERIAL_MAP_ROUGHNESS;
    const int shaderLocation = GetShaderLocation(displacement_shader, "heightMap");
    SetShaderValue(displacement_shader, shaderLocation, &heightmapSlotIndex, SHADER_UNIFORM_INT);

    constexpr float heightScale = 1.0;
    const int scaleLoc = GetShaderLocation(displacement_shader, "heightScale");
    SetShaderValue(displacement_shader, scaleLoc, &heightScale, SHADER_UNIFORM_FLOAT);
  }

  void stream(entt::registry& registry, const Camera3D& camera) {
    const auto& offset = get_player(registry).offset;
    const auto& rm = get_resource_manager(registry);

    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      if (!rm.textures.contains(chunk.model)) continue;
      if (!rm.textures.contains(chunk.height)) continue;

      auto& model = terrain_models[chunk.zoom];

      const auto tex = rm.textures[chunk.model]->res;
      const auto heightmap = rm.textures[chunk.height]->res;

      model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = tex;
      model.materials[0].maps[MATERIAL_MAP_ROUGHNESS].texture = heightmap;

      const Vector3 position = pos.pos + offset;
      DrawModel(model, position, 1.0f, WHITE);
    }
  }

  void update(entt::registry& registry) {
    const auto& player = get_player(registry);
    const TilesDef& tiles = get_tiles_def(registry);
    auto& rm = get_resource_manager(registry);

    const auto position = player.absolute_position();

    // check if we moved to a new tile at ANY zoom level
    bool any_moved = false;
    for (int i = 0; i < LOD_COUNT; ++i) {
      const auto& lod = LOD_LEVELS[i];
      const int tx = static_cast<int>(std::floor(position.x / lod.tile_size));
      const int tz = static_cast<int>(std::floor(position.z / lod.tile_size));
      if (tx != last_tile_x[i] || tz != last_tile_z[i]) {
        any_moved = true;
        last_tile_x[i] = tx;
        last_tile_z[i] = tz;
      }
    }
    if (!any_moved) return;

    // build required tile set across all zoom levels (highest zoom first)
    std::set<TileKey> required;
    for (int i = 0; i < LOD_COUNT; ++i) {
      const auto& lod = LOD_LEVELS[i];
      const int center_x = static_cast<int>(std::floor(position.x / lod.tile_size));
      const int center_z = static_cast<int>(std::floor(position.z / lod.tile_size));

      for (int dx = -lod.radius; dx <= lod.radius; ++dx) {
        for (int dz = -lod.radius; dz <= lod.radius; ++dz) {
          const int rx = center_x + dx;
          const int rz = center_z + dz;

          // skip if this tile is fully covered by higher-zoom children already in required
          if (i > 0 && is_covered(lod.zoom, rx, rz, required)) continue;

          required.insert({lod.zoom, rx, rz});
        }
      }
    }

    // determine which active tiles to evict
    for (auto it = active_tiles.begin(); it != active_tiles.end();) {
      if (!required.contains(it->first)) {
        const auto& key = it->first;
        // if this tile is being replaced, mark the old entity for graceful removal
        // (handled by process_loaded_chunks once children are ready)
        // For now: check if the entity has PendingReplacement — if so, skip destruction
        if (registry.valid(it->second) && registry.all_of<PendingReplacement>(it->second)) {
          ++it;
          continue;
        }

        if (registry.valid(it->second)) {
          registry.destroy(it->second);
        }
        const auto texture_id = get_tex_id(key.zoom, key.x, key.z);
        const auto height_id = get_height_id(key.zoom, key.x, key.z);
        rm.textures.erase(texture_id);
        rm.textures.erase(height_id);
        rm.images.erase(height_id);
        it = active_tiles.erase(it);
      } else {
        ++it;
      }
    }

    // spawn new tiles for required entries not yet active
    for (const auto& key : required) {
      if (active_tiles.contains(key)) continue;

      // check if a lower-zoom tile already covers this area — set up graceful replacement
      entt::entity old_tile = entt::null;
      TileKey old_key{};
      if (key.zoom > LOD_LEVELS[LOD_COUNT - 1].zoom) {
        // find parent tile at one zoom level lower
        const int parent_zoom = key.zoom - 1;
        const int parent_x = static_cast<int>(std::floor(static_cast<float>(key.x) / 2.0f));
        const int parent_z = static_cast<int>(std::floor(static_cast<float>(key.z) / 2.0f));
        old_key = {parent_zoom, parent_x, parent_z};
        if (auto pit = active_tiles.find(old_key); pit != active_tiles.end()) {
          if (registry.valid(pit->second) && registry.all_of<TerrainChunk>(pit->second)) {
            old_tile = pit->second;
          }
        }
      }

      const entt::entity new_entity = spawn_tile(registry, key, tiles);
      active_tiles[key] = new_entity;

      // set up graceful replacement: keep old tile alive until this new one loads
      if (old_tile != entt::null) {
        if (!registry.all_of<PendingReplacement>(old_tile)) {
          registry.emplace<PendingReplacement>(old_tile, std::vector<entt::entity>{new_entity});
        } else {
          registry.get<PendingReplacement>(old_tile).replacements.push_back(new_entity);
        }
      }
    }
  }

  void process_loaded_chunks(entt::registry& registry) {
    // process async tile loads — one per frame
    for (const auto view = registry.view<AsyncTileLoad>(); const auto [entity, tile] : view.each()) {
      const bool tex_ready = tile.texture_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
      const bool height_ready = tile.heightmap_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;

      if (!tex_ready || !height_ready) continue;

      const Image tex_img = tile.texture_future.get();
      const Image height_img = tile.heightmap_future.get();

      const Texture2D texture_tex = LoadTextureFromImage(tex_img);
      const Texture2D height_tex = LoadTextureFromImage(height_img);

      UnloadImage(tex_img);

      const auto texture_id = get_tex_id(tile.zoom, tile.x, tile.z);
      const auto height_id = get_height_id(tile.zoom, tile.x, tile.z);

      auto& rm = get_resource_manager(registry);
      rm.textures.load(texture_id, texture_tex);
      rm.textures.load(height_id, height_tex);
      rm.images.load(height_id, height_img);

      registry.remove<AsyncTileLoad>(entity);
      registry.emplace<TerrainChunk>(entity, texture_id, height_id, tile.zoom);
      registry.emplace<TerrainHeight>(entity, height_id);
      break;  // one per frame
    }

    // check pending replacements — destroy old tiles whose children are all loaded
    auto& rm = get_resource_manager(registry);
    for (const auto view = registry.view<PendingReplacement>(); const auto [entity, pending] : view.each()) {
      bool all_ready = true;
      for (const auto& child : pending.replacements) {
        if (!registry.valid(child) || !registry.all_of<TerrainChunk>(child)) {
          all_ready = false;
          break;
        }
      }
      if (!all_ready) continue;

      // all replacement children are loaded — destroy the old tile
      if (registry.all_of<TerrainChunk>(entity)) {
        const auto& chunk = registry.get<TerrainChunk>(entity);
        rm.textures.erase(chunk.model);
        rm.textures.erase(chunk.height);
        rm.images.erase(chunk.height);
      }

      // remove from active_tiles
      for (auto it = active_tiles.begin(); it != active_tiles.end(); ++it) {
        if (it->second == entity) {
          active_tiles.erase(it);
          break;
        }
      }
      registry.destroy(entity);
      break;  // one cleanup per frame to avoid iterator invalidation
    }
  }

 private:
  entt::entity spawn_tile(entt::registry& registry, const TileKey& key, const TilesDef& tiles) {
    const auto& lod = *std::find_if(std::begin(LOD_LEVELS), std::end(LOD_LEVELS), [&](const auto& l) { return l.zoom == key.zoom; });

    const auto entity = registry.create();

    const int tx = key.x + lod.base_x;
    const int tz = key.z + lod.base_z;
    TraceLog(LOG_INFO, "spawning tile z%d %d (%d), %d (%d)", key.zoom, key.x, tx, key.z, tz);

    std::string tex_path = std::vformat(tiles.tex_path, std::make_format_args(key.zoom, tx, tz));
    std::string height_path = std::vformat(tiles.hmp_path, std::make_format_args(key.zoom, tx, tz));

    auto tex_task = std::async(std::launch::async, [tex_path]() { return LoadImage(tex_path.c_str()); });

    auto height_task = std::async(std::launch::async, [height_path]() { return LoadImage(height_path.c_str()); });

    const float world_x = static_cast<float>(key.x) * lod.tile_size;
    const float world_z = static_cast<float>(key.z) * lod.tile_size;

    registry.emplace<Position3D>(entity, (Vector3){world_x, tiles.lowest, world_z}, Vector3Zero());
    registry.emplace<AsyncTileLoad>(entity, std::move(tex_task), std::move(height_task), key.zoom, key.x, key.z);

    return entity;
  }
};
}  // namespace terrain_streamer