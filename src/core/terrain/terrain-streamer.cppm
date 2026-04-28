module;
#include <entt/entt.hpp>
#include <filesystem>
#include <future>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <thread>

#include "../../lib/ray.hpp"

export module TerrainStreaming;

import Components;
import RaylibResource;
import ResourceManager;
import Resources;
import Accessors;
import Types;

constexpr Meter SKIRT_SIZE = 0.0f;
constexpr Meter TILE_SIZE = 9783.9;      // zoom 12
constexpr Meter TILE_SIZE_12 = 9783.9f;  // zoom 12
constexpr Meter TILE_SIZE_13 = 4891.95f;
constexpr Meter TILE_SIZE_14 = 2445.975f;
constexpr int ZOOM_LEVEL = 12;
constexpr int BASE_X = 2444;
constexpr int BASE_Z = 1655;

// Disc radius in z12 tile-units squared (matches existing dx*dx+dz*dz <= 30).
constexpr int RENDER_DISC_R2 = 36;
// Render distance in meters: sqrt(30) * TILE_SIZE_12 ~= 53,610 m.
constexpr Meter RENDER_RADIUS = 6 * TILE_SIZE_12;
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

Model create_model(const Meter size) { return LoadModelFromMesh(GenMeshPlane(size, size + size * 0.02f, 256, 256)); }

export struct AsyncTileLoad {
  std::future<Image> texture_future;
  std::future<Image> heightmap_future;
  int x = 0;
  int z = 0;
  int zoom = 12;
};

export struct TerrainChunk {
  int model;
  int height;
  int zoom = 12;
  int x = 0;
  int z = 0;
};

export struct TerrainHeight {
  int height;  // id of the height model
};

// Guards concurrent downloads of the same file path.
// Two async threads for the same tile (texture + heightmap) would otherwise both
// run download_tile.mjs simultaneously and race to write the same file.
namespace {
std::mutex download_mutex;
std::set<std::string> downloading;
}  // namespace

static Image load_tile_image(int zoom, int tx, int tz, const std::string& path) {
  {
    std::lock_guard lock(download_mutex);
    if (!std::filesystem::exists(path) && !downloading.contains(path)) {
      downloading.insert(path);
    } else {
      // Either already on disk or another thread is downloading — just wait for the file.
      while (!std::filesystem::exists(path)) {
        // spin with a small sleep; the other thread owns the download
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
      return LoadImage(path.c_str());
    }
  }
  // We own the download for this path.
  std::string cmd = "./download_tile.mjs " + std::to_string(zoom) + " " + std::to_string(tx) + " " + std::to_string(tz);
  std::system(cmd.c_str());
  {
    std::lock_guard lock(download_mutex);
    downloading.erase(path);
  }
  return LoadImage(path.c_str());
}

inline int get_tile_id(const int x, const int z) { return entt::hashed_string(TextFormat("tile_model_%d_%d", x, z)); }
inline int get_tex_id(int zoom, int x, int z) { return entt::hashed_string(TextFormat("tile_tex_%d_%d_%d", zoom, x, z)); }
inline int get_height_id(int zoom, int x, int z) { return entt::hashed_string(TextFormat("tile_height_%d_%d_%d", zoom, x, z)); }

export namespace terrain_streamer {

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

  void stream(entt::registry& registry, const Camera3D& camera) {
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
    std::set<TileKey> new_desired_keys;
    for (int dx = -7; dx <= 7; ++dx) {
      for (int dz = -7; dz <= 7; ++dz) {
        if (dz * dz + dx * dx > RENDER_DISC_R2) continue;
        const auto bx = current_tile_x + dx;
        const auto bz = current_tile_z + dz;

        const float world_x = (static_cast<float>(bx) + 0.5f) * TILE_SIZE_12;
        const float world_z = (static_cast<float>(bz) + 0.5f) * TILE_SIZE_12;
        const auto ddx = player_pos.x - world_x;
        const auto ddz = player_pos.z - world_z;
        const auto dist_sq = ddx * ddx + ddz * ddz;

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
        TraceLog(LOG_DEBUG, "Cancelling load z%d %d %d", key.zoom, key.x, key.z);
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
        TraceLog(LOG_DEBUG, "Unloading tile z%d %d %d", zoom, x, z);
        registry.destroy(it->second);
        auto& rm = get_resource_manager(registry);
        rm.textures.erase(get_tex_id(zoom, x, z));
        rm.textures.erase(get_height_id(zoom, x, z));
        rm.images.erase(get_height_id(zoom, x, z));
        it = rendered_tiles.erase(it);
      } else {
        ++it;  // replacements not ready yet — keep rendering
      }
    }
  }

  void process_loaded_chunks(entt::registry& registry) {
    for (const auto view = registry.view<AsyncTileLoad>(); const auto [entity, tile] : view.each()) {
      // zero wait check if the threads done
      const bool tex_ready = tile.texture_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
      const bool height_ready = tile.heightmap_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;

      // if not, we'll try again next tick
      if (!tex_ready || !height_ready) continue;
      // TraceLog(LOG_DEBUG, "processing tile z%d %d %d", tile.zoom, tile.x, tile.z);

      const Image tex_img = tile.texture_future.get();
      const Image height_img = tile.heightmap_future.get();

      // create the texture
      const Texture2D texture_tex = LoadTextureFromImage(tex_img);
      const Texture2D height_tex = LoadTextureFromImage(height_img);

      // need no more
      UnloadImage(tex_img);

      // keep textures in resource manager for the render phase
      const auto texture_id = get_tex_id(tile.zoom, tile.x, tile.z);
      const auto height_id = get_height_id(tile.zoom, tile.x, tile.z);

      // Copy coords before remove — remove<AsyncTileLoad> invalidates the tile ref.
      const int zoom = tile.zoom;
      const int tx = tile.x;
      const int tz = tile.z;

      auto& rm = get_resource_manager(registry);
      rm.textures.load(texture_id, texture_tex);
      rm.textures.load(height_id, height_tex);
      rm.images.load(height_id, height_img);

      registry.remove<AsyncTileLoad>(entity);
      registry.emplace<TerrainChunk>(entity, texture_id, height_id, zoom, tx, tz);
      registry.emplace<TerrainHeight>(entity, height_id);
      rendered_tiles[{zoom, tx, tz}] = entity;
      // break;  // to free the loop and let the next chunk load on the next frame
    }
  }

 private:
  static TileKey z12_parent(const TileKey& k) {
    const int shift = k.zoom - ZOOM_LEVEL;
    return {ZOOM_LEVEL, k.x >> shift, k.z >> shift};
  }

  bool is_parent_cell_covered(const TileKey& parent12) const {
    for (const auto& [key, _] : desired_tiles) {
      if (z12_parent(key) == parent12 && !rendered_tiles.contains(key)) return false;
    }
    return true;
  }

  entt::entity spawn_tile(entt::registry& registry, const TileKey& tile) {
    const auto entity = registry.create();

    const int scale = 1 << (tile.zoom - ZOOM_LEVEL);
    const int tx = tile.x + BASE_X * scale;
    const int tz = tile.z + BASE_Z * scale;
    TraceLog(LOG_DEBUG, "spawning tile z%d %d %d %d %d", tile.zoom, tile.x, tile.z, tx, tz);

    std::string tex_path = std::format("assets/tiles/cache/texture/{}/{}/{}.png", tile.zoom, tx, tz);
    std::string height_path = std::format("assets/tiles/cache/heightmaps/{}/{}/{}.png", tile.zoom, tx, tz);

    auto tex_task = std::async(std::launch::async, [zoom = tile.zoom, tx, tz, tex_path]() {
      return load_tile_image(zoom, tx, tz, tex_path);
    });
    auto height_task = std::async(std::launch::async, [zoom = tile.zoom, tx, tz, height_path]() {
      return load_tile_image(zoom, tx, tz, height_path);
    });

    const float tile_size = TILE_SIZE_12 / static_cast<float>(1 << (tile.zoom - ZOOM_LEVEL));
    const float world_x = (static_cast<float>(tile.x) + 0.5f) * tile_size;
    const float world_z = (static_cast<float>(tile.z) + 0.5f) * tile_size;

    registry.emplace<Position3D>(entity, (Vector3){world_x, 0.0f, world_z}, Vector3Zero());
    registry.emplace<AsyncTileLoad>(entity, std::move(tex_task), std::move(height_task), tile.x, tile.z, tile.zoom);

    return entity;
  }

  Model& model_for_zoom(const int zoom) {
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