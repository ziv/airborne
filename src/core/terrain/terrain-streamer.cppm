module;
#include <cstdlib>
#include <entt/entt.hpp>
#include <filesystem>
#include <future>
#include <map>
#include <nlohmann/json.hpp>
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

constexpr Meter SKIRT_SIZE = 0.0f;
constexpr Meter TILE_SIZE = 9783.9;      // zoom 12
constexpr Meter TILE_SIZE_12 = 9783.9f;  // zoom 12
constexpr Meter TILE_SIZE_13 = 4891.95f;
constexpr Meter TILE_SIZE_14 = 2445.975f;
constexpr int ZOOM_LEVEL = 12;
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

constexpr float SQUARE_DISTANCE_13 = 25000000.0f;  // 5_000^2
constexpr float SQUARE_DISTANCE_14 = 9000000.0f;   // 3_000^2

constexpr Meter tile_size_for_zoom(int zoom) { return TILE_SIZE_12 / static_cast<Meter>(1 << (zoom - ZOOM_LEVEL)); }

// World-space center for a tile at any zoom, aligned within its z12 parent area.
inline float tile_world_pos(const int zoom, const int local_idx) {
  const int n = 1 << (zoom - ZOOM_LEVEL);
  const int parent = local_idx >> (zoom - ZOOM_LEVEL);
  const int child = local_idx - parent * n;  // 0 .. n-1
  const Meter sz = tile_size_for_zoom(zoom);
  return static_cast<float>(parent) * TILE_SIZE_12 + (static_cast<float>(child) - (n - 1) * 0.5f) * sz;
}

// constexpr Meter TILE_SIZE = 2445.975f;  // zoom 14
// constexpr int BASE_X = 9755;
// constexpr int BASE_Z = 6627;

Model create_model(const Meter size) { return LoadModelFromMesh(GenMeshPlane(size + SKIRT_SIZE, size + SKIRT_SIZE, 256, 256)); }

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

void on_terrain_destroyed(entt::registry& reg, entt::entity entity) {
  TraceLog(LOG_DEBUG, "Destroying terrain chunk entity %d", static_cast<int>(entity));
  const auto& chunk = reg.get<TerrainChunk>(entity);
  auto& rm = get_resource_manager(reg);

  // rm.textures.erase(chunk.model);
  // rm.textures.erase(chunk.height);
  // rm.images.erase(chunk.height);
}
class streamer {
  // TilesDef& tiles;
  std::map<TileKey, entt::entity> active_tiles;
  int last_tile_x = -9999;
  int last_tile_z = -9999;
  Shader displacement_shader;
  std::unique_ptr<Model> terrain_model12;
  std::unique_ptr<Model> terrain_model13;
  std::unique_ptr<Model> terrain_model14;

 public:
  explicit streamer(entt::registry& registry)
      : displacement_shader(LoadShader("assets/shaders/terrain.vs", "assets/shaders/terrain.fs")),
        terrain_model12(std::make_unique<Model>(create_model(TILE_SIZE_12))),
        terrain_model13(std::make_unique<Model>(create_model(TILE_SIZE_13))),
        terrain_model14(std::make_unique<Model>(create_model(TILE_SIZE_14))) {
    TraceLog(LOG_DEBUG, "AAA");
    // set the displacement_shader as the terrain model shader
    terrain_model12->materials[0].shader = displacement_shader;
    terrain_model13->materials[0].shader = displacement_shader;
    terrain_model14->materials[0].shader = displacement_shader;

    TraceLog(LOG_DEBUG, "BBB");
    // set the heightmap data into MATERIAL_MAP_ROUGHNESS slot
    constexpr int heightmapSlotIndex = MATERIAL_MAP_ROUGHNESS;  // Raylib map roughness index
    const int shaderLocation = GetShaderLocation(displacement_shader, "heightMap");
    SetShaderValue(displacement_shader, shaderLocation, &heightmapSlotIndex, SHADER_UNIFORM_INT);

    // set scale 1 as long as the model is stretched properly
    constexpr float heightScale = 1.0;
    const int scaleLoc = GetShaderLocation(displacement_shader, "heightScale");
    SetShaderValue(displacement_shader, scaleLoc, &heightScale, SHADER_UNIFORM_FLOAT);

    TraceLog(LOG_DEBUG, "CCC");
    // clean resources when destroying
    // registry.on_destroy<TerrainChunk>().connect<&on_terrain_destroyed>();
  }

  static void draw_tile_labels(entt::registry& registry, const Camera3D& camera) {
    const auto& player = get_player(registry);
    const auto width = static_cast<float>(GetScreenWidth());
    const auto height = static_cast<float>(GetScreenHeight());
    // const auto& offset = get_player(registry).offset;
    // const Vector3 forward = Vector3Normalize(camera.target - camera.position);

    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      // Raise the label a bit above the ground for legibility
      const Vector3 world_pos = pos.pos + player.offset + Vector3{0.0f, 200.0f, 0.0f};

      // "In front of camera" filter via dot product with forward
      const Vector3 to_tile = world_pos - camera.position;
      if (Vector3DotProduct(to_tile, player.forward) <= 0.0f) continue;

      const Vector2 sp = GetWorldToScreen(world_pos, camera);
      if (sp.x < 0.0f || sp.x > width || sp.y < 0.0f || sp.y > height) continue;

      if (chunk.zoom == 12) DrawText(TextFormat("z%d %d,%d", chunk.zoom, chunk.x, chunk.z), static_cast<int>(sp.x), static_cast<int>(sp.y), 10, YELLOW);
    }
  }

  void stream(entt::registry& registry, const Camera3D& camera) {
    const auto& player = get_player(registry);
    const auto& rm = get_resource_manager(registry);

    // todo fog...

    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      // // the texture is not exists (not suppose to happen, just for safety)
      // if (!rm.textures.contains(chunk.model)) continue;
      // // the heightmap is not exists (not suppose to happen, just for safety)
      // if (!rm.textures.contains(chunk.height)) continue;
      //
      // // now this access is safe
      // const auto tex = rm.textures[chunk.model]->res;
      // const auto heightmap = rm.textures[chunk.height]->res;
      //
      // // attach the texture and the heightmap to the slot we defined in the ctr
      // auto& model = model_for_zoom(chunk.zoom);
      // model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = tex;
      // model.materials[0].maps[MATERIAL_MAP_ROUGHNESS].texture = heightmap;
      //
      // DrawModel(terrain_model12, pos.pos + player.offset, 1.0f, WHITE);
      if (chunk.zoom == 12) {
        DrawCube(pos.pos + player.offset, TILE_SIZE_12, 100.0f, TILE_SIZE_12, RED);
      }
      if (chunk.zoom == 13) {
        DrawCube(pos.pos + player.offset, TILE_SIZE_13, 100.0f, TILE_SIZE_13, YELLOW);
      }
      if (chunk.zoom == 14) {
        DrawCube(pos.pos + player.offset, TILE_SIZE_14, 100.0f, TILE_SIZE_14, BLACK);
      }
    }
  }

  void update(entt::registry& registry) {
    const auto& player = get_player(registry);
    // const TilesDef& tiles = get_tiles_def(registry);
    auto& rm = get_resource_manager(registry);

    // player absolute position
    const auto player_pos = player.absolute_position();

    // the current tile based on position
    // const auto tile_world_size = tiles.meter_to_pixel * tiles.tex_size;
    const int current_tile_x = static_cast<int>(std::floor(player_pos.x / TILE_SIZE));
    const int current_tile_z = static_cast<int>(std::floor(player_pos.z / TILE_SIZE));

    // we are on the same tile as before, bye bye...
    // if (current_tile_x == last_tile_x && current_tile_z == last_tile_z) return;

    // prepare list of required tiles
    // std::vector<TileKey> required;
    // required.reserve(256);

    std::set<TileKey> required_set;

    for (int dx = -6; dx <= 6; ++dx) {
      for (int dz = -6; dz <= 6; ++dz) {
        if (dz * dz + dx * dx > RENDER_DISC_R2) continue;
        const auto bx = current_tile_x + dx;
        const auto bz = current_tile_z + dz;

        const float world_x = (static_cast<float>(bx) + 0.5f) * TILE_SIZE_12;
        const float world_z = (static_cast<float>(bz) + 0.5f) * TILE_SIZE_12;

        // Distance from player to this z12 cell center
        // const auto cx12 = tile_world_pos(ZOOM_LEVEL, bx);
        // const auto cz12 = tile_world_pos(ZOOM_LEVEL, bz);
        const auto ddx = player_pos.x - world_x;
        const auto ddz = player_pos.z - world_z;
        const auto dist_sq = ddx * ddx + ddz * ddz;

        if (dist_sq < Z14_THRESHOLD_SQ) {
          // 16 z14 children
          const int cx0 = bx * 4;
          const int cz0 = bz * 4;
          for (int ox = 0; ox < 4; ++ox)
            for (int oz = 0; oz < 4; ++oz) required_set.insert({14, cx0 + ox, cz0 + oz});
        } else if (dist_sq < Z13_THRESHOLD_SQ) {
          // 4 z13 children
          const int cx0 = bx * 2;
          const int cz0 = bz * 2;
          for (int ox = 0; ox < 2; ++ox)
            for (int oz = 0; oz < 2; ++oz) required_set.insert({13, cx0 + ox, cz0 + oz});
        } else {
          required_set.insert({12, bx, bz});
        }
      }
    }

    // const std::set<TileKey> required_set(required.begin(), required.end());

    // For each z12 parent area touched by the required set, count how many of
    // the required tiles in that area are already loaded (have TerrainChunk).
    // A stale tile is only safe to unload when:
    //   (a) no required tile shares its z12 parent (player moved away), or
    //   (b) every required tile in its z12 parent area is fully loaded.
    // using ParentCoord = std::pair<int, int>;
    // struct ParentStatus {
    //   int total = 0;
    //   int loaded = 0;
    // };
    // std::map<ParentCoord, ParentStatus> parent_status;
    // for (const auto& key : required_set) {
    //   const int shift = key.zoom - ZOOM_LEVEL;
    //   const ParentCoord parent{key.x >> shift, key.z >> shift};
    //   auto& st = parent_status[parent];
    //   st.total++;
    //   if (auto it = active_tiles.find(key); it != active_tiles.end()) {
    //     if (registry.all_of<TerrainChunk>(it->second)) st.loaded++;
    //   }
    // }

    // iterating active and remove tiles not on required
    for (auto it = active_tiles.begin(); it != active_tiles.end();) {
      if (required_set.contains(it->first)) {
        ++it;
        continue;
      }
      const auto& [zoom, x, z] = it->first;
      TraceLog(LOG_DEBUG, "Unloading tile z%d %d %d", zoom, x, z);
      registry.destroy(it->second);
      rm.textures.erase(get_tex_id(zoom, x, z));
      rm.textures.erase(get_height_id(zoom, x, z));
      rm.images.erase(get_height_id(zoom, x, z));
      it = active_tiles.erase(it);
    }
    // for (auto it = active_tiles.begin(); it != active_tiles.end();) {
    //   if (required_set.contains(it->first)) {
    //     ++it;
    //     continue;
    //   }
    //   const auto& [zoom, x, z] = it->first;
    //   TraceLog(LOG_DEBUG, "Unloading tile z%d %d %d", zoom, x, z);
    //   registry.destroy(it->second);
    //   rm.textures.erase(get_tex_id(zoom, x, z));
    //   rm.textures.erase(get_height_id(zoom, x, z));
    //   rm.images.erase(get_height_id(zoom, x, z));
    //   it = active_tiles.erase(it);
    // }

    // iterate the required tile and find not loaded
    for (const auto& key : required_set) {
      if (!active_tiles.contains(key)) {
        const entt::entity entity = spawn_tile(registry, key);
        active_tiles[key] = entity;
      }
    }

    last_tile_x = current_tile_x;
    last_tile_z = current_tile_z;
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

      auto& rm = get_resource_manager(registry);
      rm.textures.load(texture_id, texture_tex);
      rm.textures.load(height_id, height_tex);
      rm.images.load(height_id, height_img);

      registry.remove<AsyncTileLoad>(entity);
      registry.emplace<TerrainChunk>(entity, texture_id, height_id, tile.zoom, tile.x, tile.z);
      registry.emplace<TerrainHeight>(entity, height_id);
      break;  // to free the loop and let the next chunk load on the next frame
    }
  }

 private:
  entt::entity spawn_tile(entt::registry& registry, const TileKey& tile) {
    const auto entity = registry.create();

    const int scale = 1 << (tile.zoom - ZOOM_LEVEL);
    const int tx = tile.x + BASE_X * scale;
    const int tz = tile.z + BASE_Z * scale;
    TraceLog(LOG_DEBUG, "spawning tile z%d %d %d %d %d", tile.zoom, tile.x, tile.z, tx, tz);

    std::string tex_path = std::format("assets/tiles/cache/texture/{}/{}/{}.png", tile.zoom, tx, tz);
    std::string height_path = std::format("assets/tiles/cache/heightmaps/{}/{}/{}.png", tile.zoom, tx, tz);

    auto ensure_tile = [](int zoom, int tx, int tz, const std::string& path) {
      if (!std::filesystem::exists(path)) {
        std::string cmd = "./download_tile.mjs " + std::to_string(zoom) + " " + std::to_string(tx) + " " + std::to_string(tz);
        std::system(cmd.c_str());
      }
      return LoadImage(path.c_str());
    };

    auto tex_task = std::async(std::launch::async, [ensure_tile, zoom = tile.zoom, tx, tz, tex_path]() { return ensure_tile(zoom, tx, tz, tex_path); });

    auto height_task =
        std::async(std::launch::async, [ensure_tile, zoom = tile.zoom, tx, tz, height_path]() { return ensure_tile(zoom, tx, tz, height_path); });

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