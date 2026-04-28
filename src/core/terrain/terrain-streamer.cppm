module;
#include <cmath>
#include <cstdlib>
#include <entt/entt.hpp>
#include <filesystem>
#include <format>
#include <future>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

#include "../../lib/ray.hpp"

export module TerrainStreaming;

import Components;
import RaylibResource;
import ResourceManager;
import Resources;
import Accessors;
import Types;

constexpr int BASE_ZOOM = 12;
constexpr Meter TILE_SIZE_12 = 9783.9f;
constexpr Meter TILE_SIZE_13 = TILE_SIZE_12 / 2.0f;
constexpr Meter TILE_SIZE_14 = TILE_SIZE_12 / 4.0f;
constexpr int BASE_X = 2444;
constexpr int BASE_Z = 1655;

// Disc radius in z12 tile-units squared (matches existing dx*dx+dz*dz <= 30).
constexpr int RENDER_DISC_R2 = 30;
// Render distance in meters: sqrt(30) * TILE_SIZE_12 ~= 53,610 m.
constexpr Meter RENDER_RADIUS = 5.477225575f * TILE_SIZE_12;
constexpr Meter Z13_THRESHOLD = RENDER_RADIUS * 0.5f;
constexpr Meter Z14_THRESHOLD = RENDER_RADIUS * 0.25f;
constexpr Meter Z13_THRESHOLD_SQ = Z13_THRESHOLD * Z13_THRESHOLD;
constexpr Meter Z14_THRESHOLD_SQ = Z14_THRESHOLD * Z14_THRESHOLD;

constexpr Meter tile_size_for_zoom(int zoom) { return TILE_SIZE_12 / static_cast<Meter>(1 << (zoom - BASE_ZOOM)); }

// World-space center for a tile at any zoom, aligned within its z12 parent area.
inline float tile_world_pos(int zoom, int local_idx) {
  const int n = 1 << (zoom - BASE_ZOOM);
  const int parent = local_idx >> (zoom - BASE_ZOOM);
  const int child = local_idx - parent * n;  // 0 .. n-1
  const Meter sz = tile_size_for_zoom(zoom);
  return static_cast<float>(parent) * TILE_SIZE_12 + (static_cast<float>(child) - (n - 1) * 0.5f) * sz;
}

Model create_terrain_model(Meter size) { return LoadModelFromMesh(GenMeshPlane(size, size, 256, 256)); }

export struct TileKey {
  int zoom;
  int x;
  int z;
  auto operator<=>(const TileKey&) const = default;
};

export struct AsyncTileLoad {
  std::future<Image> texture_future;
  std::future<Image> heightmap_future;
  int zoom = BASE_ZOOM;
  int x = 0;
  int z = 0;
};

export struct TerrainChunk {
  int model;
  int height;
  int zoom = BASE_ZOOM;
  int x = 0;
  int z = 0;
};

export struct TerrainHeight {
  int height;  // id of the height model
};

inline int get_tex_id(int zoom, int x, int z) { return entt::hashed_string(TextFormat("tile_tex_%d_%d_%d", zoom, x, z)); }
inline int get_height_id(int zoom, int x, int z) { return entt::hashed_string(TextFormat("tile_height_%d_%d_%d", zoom, x, z)); }

export namespace terrain_streamer {

void on_terrain_destroyed(entt::registry& reg, entt::entity entity) { TraceLog(LOG_INFO, "Destroying terrain chunk entity %d", static_cast<int>(entity)); }

class streamer {
  std::map<TileKey, entt::entity> active_tiles;
  Shader displacement_shader;
  Model terrain_model_z12;
  Model terrain_model_z13;
  Model terrain_model_z14;

  Model& model_for_zoom(int zoom) {
    switch (zoom) {
      case 13:
        return terrain_model_z13;
      case 14:
        return terrain_model_z14;
      default:
        return terrain_model_z12;
    }
  }

 public:
  explicit streamer(entt::registry& registry)
      : displacement_shader(LoadShader("assets/shaders/terrain.vs", "assets/shaders/terrain.fs")),
        terrain_model_z12(create_terrain_model(TILE_SIZE_12)),
        terrain_model_z13(create_terrain_model(TILE_SIZE_13)),
        terrain_model_z14(create_terrain_model(TILE_SIZE_14)) {
    terrain_model_z12.materials[0].shader = displacement_shader;
    terrain_model_z13.materials[0].shader = displacement_shader;
    terrain_model_z14.materials[0].shader = displacement_shader;

    constexpr int heightmapSlotIndex = MATERIAL_MAP_ROUGHNESS;  // Raylib map roughness slot
    const int shaderLocation = GetShaderLocation(displacement_shader, "heightMap");
    SetShaderValue(displacement_shader, shaderLocation, &heightmapSlotIndex, SHADER_UNIFORM_INT);

    constexpr float heightScale = 1.0f;
    const int scaleLoc = GetShaderLocation(displacement_shader, "heightScale");
    SetShaderValue(displacement_shader, scaleLoc, &heightScale, SHADER_UNIFORM_FLOAT);
  }

  void stream(entt::registry& registry, const Camera3D& camera) {
    const auto& offset = get_player(registry).offset;
    const auto& rm = get_resource_manager(registry);

    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      if (!rm.textures.contains(chunk.model)) continue;
      if (!rm.textures.contains(chunk.height)) continue;

      const auto tex = rm.textures[chunk.model]->res;
      const auto heightmap = rm.textures[chunk.height]->res;

      auto& model = model_for_zoom(chunk.zoom);
      model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = tex;
      model.materials[0].maps[MATERIAL_MAP_ROUGHNESS].texture = heightmap;

      const Vector3 position = pos.pos + offset;
      DrawModel(model, position, 1.0f, WHITE);
    }
  }

  // 2D pass: draw a small label at each tile center showing its zoom and (x,z).
  // Only labels tiles in front of the camera.
  void draw_tile_labels(entt::registry& registry, const Camera3D& camera) {
    const auto& offset = get_player(registry).offset;
    const Vector3 forward = Vector3Normalize(camera.target - camera.position);

    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      // Raise the label a bit above the ground for legibility
      const Vector3 world_pos = pos.pos + offset + Vector3{0.0f, 200.0f, 0.0f};

      // "In front of camera" filter via dot product with forward
      const Vector3 to_tile = world_pos - camera.position;
      if (Vector3DotProduct(to_tile, forward) <= 0.0f) continue;

      const Vector2 sp = GetWorldToScreen(world_pos, camera);
      if (sp.x < 0 || sp.x > GetScreenWidth() || sp.y < 0 || sp.y > GetScreenHeight()) continue;

      // const char* label = TextFormat("z%d %d,%d", chunk.zoom, chunk.x, chunk.z);
      // const int tw = MeasureText(label, 10);
      // DrawRectangle(static_cast<int>(sp.x) - tw / 2 - 2, static_cast<int>(sp.y) - 1, tw + 4, 12, Fade(BLACK, 0.5f));
      DrawText(TextFormat("z%d %d,%d", chunk.zoom, chunk.x, chunk.z), static_cast<int>(sp.x), static_cast<int>(sp.y), 10, YELLOW);
    }
  }

  void update(entt::registry& registry) {
    const auto& player = get_player(registry);
    auto& rm = get_resource_manager(registry);

    const Vector3 player_pos = player.absolute_position();

    const int current_tile_x = static_cast<int>(std::floor(player_pos.x / TILE_SIZE_12));
    const int current_tile_z = static_cast<int>(std::floor(player_pos.z / TILE_SIZE_12));

    // 1) Build z12 disc list (same as before).
    std::vector<TileKey> required;
    required.reserve(128);
    for (int dx = -6; dx <= 6; ++dx) {
      for (int dz = -6; dz <= 6; ++dz) {
        if (dx * dx + dz * dz > RENDER_DISC_R2) continue;
        required.push_back({BASE_ZOOM, current_tile_x + dx, current_tile_z + dz});
      }
    }

    auto dist_sq_to_tile = [&](int zoom, int tx, int tz) {
      const float cx = tile_world_pos(zoom, tx);
      const float cz = tile_world_pos(zoom, tz);
      const float dx = player_pos.x - cx;
      const float dz = player_pos.z - cz;
      return dx * dx + dz * dz;
    };

    auto split_pass = [&](int from_zoom, Meter threshold_sq) {
      std::vector<TileKey> next;
      next.reserve(required.size() * 2);
      for (const auto& key : required) {
        if (key.zoom != from_zoom) {
          next.push_back(key);
          continue;
        }
        if (dist_sq_to_tile(key.zoom, key.x, key.z) >= threshold_sq) {
          next.push_back(key);
          continue;
        }
        // Replace with 4 children at zoom+1.
        const int child_zoom = from_zoom + 1;
        const int cx0 = key.x * 2;
        const int cz0 = key.z * 2;
        next.push_back({child_zoom, cx0, cz0});
        next.push_back({child_zoom, cx0 + 1, cz0});
        next.push_back({child_zoom, cx0, cz0 + 1});
        next.push_back({child_zoom, cx0 + 1, cz0 + 1});
      }
      required = std::move(next);
    };

    // 2) Pass 1: z12 -> z13.
    split_pass(12, Z13_THRESHOLD_SQ);
    // 3) Pass 2: z13 -> z14.
    split_pass(13, Z14_THRESHOLD_SQ);

    const std::set<TileKey> required_set(required.begin(), required.end());

    // 4) Remove tiles no longer required.
    for (auto it = active_tiles.begin(); it != active_tiles.end();) {
      if (!required_set.contains(it->first)) {
        TraceLog(LOG_INFO, "Unloading tile z%d/%d/%d", it->first.zoom, it->first.x, it->first.z);
        registry.destroy(it->second);
        const auto& key = it->first;
        rm.textures.erase(get_tex_id(key.zoom, key.x, key.z));
        rm.textures.erase(get_height_id(key.zoom, key.x, key.z));
        rm.images.erase(get_height_id(key.zoom, key.x, key.z));
        it = active_tiles.erase(it);
      } else {
        ++it;
      }
    }

    // 5) Spawn missing tiles.
    for (const auto& key : required_set) {
      if (!active_tiles.contains(key)) {
        const entt::entity entity = spawn_tile(registry, key);
        active_tiles[key] = entity;
      }
    }
  }

  void process_loaded_chunks(entt::registry& registry) {
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
      registry.emplace<TerrainChunk>(entity, texture_id, height_id, tile.zoom, tile.x, tile.z);
      registry.emplace<TerrainHeight>(entity, height_id);
      break;  // one chunk per frame
    }
  }

 private:
  entt::entity spawn_tile(entt::registry& registry, const TileKey& key) {
    const auto entity = registry.create();
    const int scale = 1 << (key.zoom - BASE_ZOOM);
    const int tx = key.x + BASE_X * scale;
    const int tz = key.z + BASE_Z * scale;

    TraceLog(LOG_WARNING, "spawning tile z%d local(%d,%d) mapbox(%d,%d)", key.zoom, key.x, key.z, tx, tz);

    const std::string tex_path = std::format("assets/tiles/cache/texture/{}/{}/{}.png", key.zoom, tx, tz);
    const std::string hmp_path = std::format("assets/tiles/cache/heightmaps/{}/{}/{}.png", key.zoom, tx, tz);

    auto ensure_tile = [](int zoom, int atx, int atz, const std::string& path) {
      if (!std::filesystem::exists(path)) {
        const std::string cmd = "./download_tile.mjs " + std::to_string(zoom) + " " + std::to_string(atx) + " " + std::to_string(atz);
        std::system(cmd.c_str());
      }
      return LoadImage(path.c_str());
    };

    auto tex_task = std::async(std::launch::async, [ensure_tile, zoom = key.zoom, tx, tz, tex_path]() { return ensure_tile(zoom, tx, tz, tex_path); });
    auto hmp_task = std::async(std::launch::async, [ensure_tile, zoom = key.zoom, tx, tz, hmp_path]() { return ensure_tile(zoom, tx, tz, hmp_path); });

    const float world_x = tile_world_pos(key.zoom, key.x);
    const float world_z = tile_world_pos(key.zoom, key.z);

    registry.emplace<Position3D>(entity, (Vector3){world_x, 0.0f, world_z}, Vector3Zero());
    registry.emplace<AsyncTileLoad>(entity, std::move(tex_task), std::move(hmp_task), key.zoom, key.x, key.z);

    return entity;
  }
};
}  // namespace terrain_streamer