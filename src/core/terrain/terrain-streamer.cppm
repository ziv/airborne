module;
#include <algorithm>
#include <entt/entt.hpp>
#include <format>
#include <future>
#include <map>
#include <set>
#include <string>

#include "../../lib/ray.hpp"

export module TerrainStreaming;

import Components;
import RaylibResource;
import ResourceManager;
import Resources;
import Accessors;
import Types;
import TileDownloader;

constexpr Meter SKIRT_SIZE = 0.0f;
constexpr Meter TILE_SIZE = 9783.9;      // zoom 12
constexpr Meter TILE_SIZE_12 = 9783.9f;  // zoom 12
constexpr Meter TILE_SIZE_13 = 4891.95f;
constexpr Meter TILE_SIZE_14 = 2445.975f;
constexpr int ZOOM_LEVEL = 12;
constexpr int BASE_X = 2444;
constexpr int BASE_Z = 1655;

// Disc radius in z12 tile-units squared (matches existing dx*dx+dz*dz <= 30).
constexpr int RENDER_DISC_R2 = 32;
// Render distance in meters: sqrt(30) * TILE_SIZE_12 ~= 53,610 m.
constexpr Meter RENDER_RADIUS = 5.6568f * TILE_SIZE_12;
constexpr Meter Z13_THRESHOLD = RENDER_RADIUS * 0.5f;
constexpr Meter Z14_THRESHOLD = RENDER_RADIUS * 0.25f;
constexpr Meter Z13_THRESHOLD_SQ = Z13_THRESHOLD * Z13_THRESHOLD;
constexpr Meter Z14_THRESHOLD_SQ = Z14_THRESHOLD * Z14_THRESHOLD;

constexpr Meter tile_size_for_zoom(int zoom) { return TILE_SIZE_12 / static_cast<Meter>(1 << (zoom - ZOOM_LEVEL)); }

// World-space center for a tile at any zoom, aligned within its z12 parent area.
inline float tile_world_pos(const int zoom, const int local_idx) {
  const int n = 1 << (zoom - ZOOM_LEVEL);
  const int parent = local_idx >> (zoom - ZOOM_LEVEL);
  const int child = local_idx - parent * n;  // 0 .. n-1
  const Meter sz = tile_size_for_zoom(zoom);
  return static_cast<float>(parent) * TILE_SIZE_12 + (static_cast<float>(child) - (static_cast<float>(n) - 1.0f) * 0.5f) * sz;
}

Model create_model(const Meter size) { return LoadModelFromMesh(GenMeshPlane(size + size * 0.02f, size + size * 0.02f, 256, 256)); }

export struct AsyncTileLoad {
  std::shared_future<void> texture_future;
  std::shared_future<void> heightmap_future;
  std::string tex_path;
  std::string height_path;
  int x = 0;
  int z = 0;
  int zoom = 12;
};

export struct TerrainChunk {
  int model{};
  int height{};
  int zoom = 12;
  int x = 0;
  int z = 0;
};

export struct TerrainHeight {
  int height;  // id of the height model
};

int get_tex_id(int zoom, int x, int z) { return entt::hashed_string(TextFormat("tile_tex_%d_%d_%d", zoom, x, z)); }
int get_height_id(int zoom, int x, int z) { return entt::hashed_string(TextFormat("tile_height_%d_%d_%d", zoom, x, z)); }

void unload_tile(ResourceManager& rm, const int zoom, const int x, const int z) {
  rm.textures.erase(get_tex_id(zoom, x, z));
  rm.textures.erase(get_height_id(zoom, x, z));
  rm.images.erase(get_height_id(zoom, x, z));
}

export namespace terrain_streamer {

// Returns the terrain elevation (metres, Mapbox RGB encoding) at a world-space
// XZ position by sampling the highest-resolution loaded tile that covers it.
// Falls back through z14 → z13 → z12 so it works while tiles are still loading.
float ground_height_at(entt::registry& registry, const Vector3& pos) {
  auto& rm = get_resource_manager(registry);
  for (const int zoom : {14, 13, 12}) {
    const float tile_size = TILE_SIZE_12 / static_cast<float>(1 << (zoom - ZOOM_LEVEL));
    const int tx = static_cast<int>(std::floor(pos.x / tile_size));
    const int tz = static_cast<int>(std::floor(pos.z / tile_size));

    const int height_id = get_height_id(zoom, tx, tz);
    if (!rm.images.contains(height_id)) continue;

    const Image& img = rm.images[height_id]->res;
    const float u = pos.x / tile_size - static_cast<float>(tx);
    const float v = pos.z / tile_size - static_cast<float>(tz);
    const int px = std::clamp(static_cast<int>(u * static_cast<float>(img.width)), 0, img.width - 1);
    const int pz = std::clamp(static_cast<int>(v * static_cast<float>(img.height)), 0, img.height - 1);

    const auto c = GetImageColor(img, px, pz);
    return -10000.0f + (static_cast<float>(c.r) * 65536.0f + static_cast<float>(c.g) * 256.0f + static_cast<float>(c.b)) * 0.1f;
  }
  return 0.0f;  // no tile loaded yet
}

struct TileKey {
  int zoom;
  int x;
  int z;
  auto operator<=>(const TileKey&) const = default;
};

class streamer {
  std::map<TileKey, entt::entity> desired_tiles;   // what LOD logic wants this frame
  std::map<TileKey, entt::entity> rendered_tiles;  // superset: desired + pending eviction
  Vector3 last_position{-9.9f, -9.9f, -9.9f};
  Shader displacement_shader;
  int cam_pos_loc = -1;
  std::unique_ptr<Model> terrain_model12;
  std::unique_ptr<Model> terrain_model13;
  std::unique_ptr<Model> terrain_model14;

 public:
  explicit streamer(entt::registry& registry)
      : displacement_shader(LoadShader("assets/shaders/terrain.vs", "assets/shaders/terrain.fs")),
        terrain_model12(std::make_unique<Model>(create_model(TILE_SIZE_12))),
        terrain_model13(std::make_unique<Model>(create_model(TILE_SIZE_13))),
        terrain_model14(std::make_unique<Model>(create_model(TILE_SIZE_14))) {
    // set the displacement_shader as the terrain model shader
    terrain_model12->materials[0].shader = displacement_shader;
    terrain_model13->materials[0].shader = displacement_shader;
    terrain_model14->materials[0].shader = displacement_shader;

    // set the heightmap data into MATERIAL_MAP_ROUGHNESS slot
    constexpr int heightmapSlotIndex = MATERIAL_MAP_ROUGHNESS;  // Raylib map roughness index
    const int shaderLocation = GetShaderLocation(displacement_shader, "heightMap");
    SetShaderValue(displacement_shader, shaderLocation, &heightmapSlotIndex, SHADER_UNIFORM_INT);

    // set scale 1 as long as the model is stretched properly
    constexpr float heightScale = 1.0;
    const int scaleLoc = GetShaderLocation(displacement_shader, "heightScale");
    SetShaderValue(displacement_shader, scaleLoc, &heightScale, SHADER_UNIFORM_FLOAT);

    cam_pos_loc = GetShaderLocation(displacement_shader, "cameraPosition");
  }

  static void draw_tile_labels(entt::registry& registry, const Camera3D& camera) {
    const auto& player = get_player(registry);
    const auto width = static_cast<float>(GetScreenWidth());
    const auto height = static_cast<float>(GetScreenHeight());

    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      // Raise the label a bit above the ground for legibility
      const Vector3 world_pos = pos.pos + player.offset + Vector3{0.0f, 300.0f, 0.0f};

      // "In front of camera" filter via dot product with forward
      const Vector3 to_tile = world_pos - camera.position;
      if (Vector3DotProduct(to_tile, player.forward) <= 0.0f) continue;

      const Vector2 sp = GetWorldToScreen(world_pos, camera);
      if (sp.x < 0.0f || sp.x > width || sp.y < 0.0f || sp.y > height) continue;

      DrawText(TextFormat("%d", chunk.zoom), static_cast<int>(sp.x), static_cast<int>(sp.y), 10, YELLOW);
    }
  }

  void stream(entt::registry& registry, const Camera3D& camera) const {
    const auto& player = get_player(registry);
    const auto& rm = get_resource_manager(registry);

    SetShaderValue(displacement_shader, cam_pos_loc, &camera.position, SHADER_UNIFORM_VEC3);

    BeginBlendMode(BLEND_ALPHA);
    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      // the texture is not exists (not suppose to happen, just for safety)
      if (!rm.textures.contains(chunk.model)) continue;
      // the heightmap is not exists (not suppose to happen, just for safety)
      if (!rm.textures.contains(chunk.height)) continue;

      // now this access is safe
      const auto tex = rm.textures[chunk.model]->res;
      const auto heightmap = rm.textures[chunk.height]->res;

      // attach the texture and the heightmap to the slot we defined in the ctr
      const auto& model = model_for_zoom(chunk.zoom);
      model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = tex;
      model.materials[0].maps[MATERIAL_MAP_ROUGHNESS].texture = heightmap;

      DrawModel(model, pos.pos + player.offset, 1.0f, WHITE);
    }
    EndBlendMode();
  }

  void update(entt::registry& registry) {
    const auto& player = get_player(registry);

    const auto player_pos = player.absolute_position();

    if (Vector3DistanceSqr(player_pos, last_position) < 200.0f * 200.0f) return;  // only update when moved more than 200m
    last_position = player_pos;

    const int current_tile_x = static_cast<int>(std::floor(player_pos.x / TILE_SIZE));
    const int current_tile_z = static_cast<int>(std::floor(player_pos.z / TILE_SIZE));

    // --- Step 1: build new desired set (keys only) ---
    // todo for performance, we should use fixed size data structure
    std::set<TileKey> new_desired_keys;
    for (int dx = -7; dx <= 7; ++dx) {
      for (int dz = -7; dz <= 7; ++dz) {
        if (dz * dz + dx * dx > RENDER_DISC_R2) continue;
        const int bx = current_tile_x + dx;
        const int bz = current_tile_z + dz;

        const float world_x = (static_cast<float>(bx) + 0.5f) * TILE_SIZE_12;
        const float world_z = (static_cast<float>(bz) + 0.5f) * TILE_SIZE_12;
        const float ddx = player_pos.x - world_x;
        const float ddz = player_pos.z - world_z;
        const float dist_sq = ddx * ddx + ddz * ddz;

        if (dist_sq < Z14_THRESHOLD_SQ) {
          const int cx0 = bx * 4;
          const int cz0 = bz * 4;
          for (int ox = 0; ox < 4; ++ox)
            for (int oz = 0; oz < 4; ++oz) new_desired_keys.insert({14, cx0 + ox, cz0 + oz});
        } else if (dist_sq < Z13_THRESHOLD_SQ) {
          const int cx0 = bx * 2;
          const int cz0 = bz * 2;
          for (int ox = 0; ox < 2; ++ox)
            for (int oz = 0; oz < 2; ++oz) new_desired_keys.insert({13, cx0 + ox, cz0 + oz});
        } else {
          new_desired_keys.insert({12, bx, bz});
        }
      }
    }

    // --- Step 2: spawn newly desired tiles ---
    for (const auto& key : new_desired_keys) {
      if (!desired_tiles.contains(key)) {
        desired_tiles[key] = spawn_tile(registry, key);
      }
    }

    // --- Step 3: remove tiles no longer desired from desired_tiles ---
    for (auto it = desired_tiles.begin(); it != desired_tiles.end();) {
      if (new_desired_keys.contains(it->first)) {
        ++it;
        continue;
      }
      const auto& [key, entity] = *it;
      if (registry.all_of<AsyncTileLoad>(entity)) {
        // Still loading, never rendered — safe to cancel immediately.
        TraceLog(LOG_DEBUG, "cancelling load z%d %d %d", key.zoom, key.x, key.z);
        registry.destroy(entity);
      }
      // If it has TerrainChunk it's in rendered_tiles — leave it there for the
      // eviction check below. If it has neither (shouldn't happen) just drop it.
      it = desired_tiles.erase(it);
    }

    // --- Step 4: evict rendered tiles whose coverage is now complete ---
    for (auto it = rendered_tiles.begin(); it != rendered_tiles.end();) {
      if (desired_tiles.contains(it->first)) {
        ++it;  // still desired — keep
        continue;
      }
      // Not desired anymore. Check if every replacement tile is rendered.
      if (is_parent_cell_covered(z12_parent(it->first))) {
        const auto& [zoom, x, z] = it->first;
        TraceLog(LOG_DEBUG, "unloading tile z%d %d %d", zoom, x, z);
        registry.destroy(it->second);
        unload_tile(get_resource_manager(registry), zoom, x, z);
        it = rendered_tiles.erase(it);
      } else {
        ++it;  // replacements not ready yet — keep rendering
      }
    }
  }

  void process_loaded_chunks(entt::registry& registry) {
    for (const auto view = registry.view<AsyncTileLoad>(); const auto [entity, tile] : view.each()) {
      const bool tex_ready    = tile.texture_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
      const bool height_ready = tile.heightmap_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;

      if (!tex_ready || !height_ready) continue;

      const Image tex_img    = LoadImage(tile.tex_path.c_str());
      const Image height_img = LoadImage(tile.height_path.c_str());

      const Texture2D texture_tex = LoadTextureFromImage(tex_img);
      const Texture2D height_tex  = LoadTextureFromImage(height_img);

      UnloadImage(tex_img);

      const auto texture_id = get_tex_id(tile.zoom, tile.x, tile.z);
      const auto height_id  = get_height_id(tile.zoom, tile.x, tile.z);

      const int zoom = tile.zoom;
      const int tx   = tile.x;
      const int tz   = tile.z;

      auto& rm = get_resource_manager(registry);
      rm.textures.load(texture_id, texture_tex);
      rm.textures.load(height_id, height_tex);
      rm.images.load(height_id, height_img);

      registry.remove<AsyncTileLoad>(entity);
      registry.emplace<TerrainChunk>(entity, texture_id, height_id, zoom, tx, tz);
      registry.emplace<TerrainHeight>(entity, height_id);
      rendered_tiles[{zoom, tx, tz}] = entity;
      break;
    }
  }

 private:
  static TileKey z12_parent(const TileKey& k) {
    const int shift = k.zoom - ZOOM_LEVEL;
    return {ZOOM_LEVEL, k.x >> shift, k.z >> shift};
  }

  // [[nodiscard]] bool is_parent_cell_covered(const TileKey& parent12) const {
  //   for (const auto& key : desired_tiles | std::views::keys) {
  //     if (z12_parent(key) == parent12 && !rendered_tiles.contains(key)) return false;
  //   }
  //   return true;
  // }

  [[nodiscard]] bool is_parent_cell_covered(const TileKey& parent12) const {
    return std::ranges::all_of(desired_tiles | std::views::keys, [&](const auto& key) { return z12_parent(key) != parent12 || rendered_tiles.contains(key); });
  }

  static entt::entity spawn_tile(entt::registry& registry, const TileKey& tile) {
    const auto entity = registry.create();

    const int scale = 1 << (tile.zoom - ZOOM_LEVEL);
    const int tx = tile.x + BASE_X * scale;
    const int tz = tile.z + BASE_Z * scale;
    TraceLog(LOG_DEBUG, "spawning tile z%d %d %d %d %d", tile.zoom, tile.x, tile.z, tx, tz);

    std::string tex_path    = std::format("assets/tiles/texture/{}/{}/{}.png", tile.zoom, tx, tz);
    std::string height_path = std::format("assets/tiles/heightmaps/{}/{}/{}.png", tile.zoom, tx, tz);

    const auto mapbox_token = std::string(std::getenv("MAPBOX_TOKEN") ? std::getenv("MAPBOX_TOKEN") : "");
    const std::string tex_url    = tile_downloader::texture_url(tile.zoom, tx, tz, mapbox_token);
    const std::string height_url = tile_downloader::heightmap_url(tile.zoom, tx, tz, mapbox_token);

    auto tex_future    = tile_downloader::enqueue(tex_path, tex_url);
    auto height_future = tile_downloader::enqueue(height_path, height_url);

    const float tile_size = TILE_SIZE_12 / static_cast<float>(1 << (tile.zoom - ZOOM_LEVEL));
    const float world_x = (static_cast<float>(tile.x) + 0.5f) * tile_size;
    const float world_z = (static_cast<float>(tile.z) + 0.5f) * tile_size;

    registry.emplace<Position3D>(entity, (Vector3){world_x, 0.0f, world_z}, Vector3Zero());
    registry.emplace<AsyncTileLoad>(entity, std::move(tex_future), std::move(height_future),
                                    std::move(tex_path), std::move(height_path), tile.x, tile.z, tile.zoom);

    return entity;
  }

  [[nodiscard]] Model& model_for_zoom(const int zoom) const {
    switch (zoom) {
      case 13:
        return *terrain_model13;
      case 14:
        return *terrain_model14;
      default:
        return *terrain_model12;
    }
  }
};
}  // namespace terrain_streamer