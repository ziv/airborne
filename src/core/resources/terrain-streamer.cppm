module;
#include <entt/entt.hpp>
#include <future>
#include <map>
#include <set>
#include <string>

#include "../../lib/ray.hpp"

export module TerrainStreaming;

import Components;
import RaylibResource;
import ResourceManager;
import Accessors;

// zoom 12/north tile
inline constexpr float METERS_PER_PIXEL = 8.05f;
inline constexpr float TILE_PIXELS = 1024.0f;
// inline constexpr float HM_PIXELS = 512.0f;
inline constexpr float TILE_WORLD_SIZE = TILE_PIXELS * METERS_PER_PIXEL;
inline constexpr float TILE_OVERLAP = TILE_WORLD_SIZE / 255.0;
inline constexpr float LOWEST = -440.0f;   // Dead Sea
inline constexpr float HIGHEST = 2814.0f;  // Mount Hermon
inline constexpr int X_TILES = 33;
inline constexpr int Z_TILES = 27;

export struct AsyncTileLoad {
  std::future<Image> texture_future;
  std::future<Image> heightmap_future;
  int x = 0;
  int z = 0;
};

export struct TerrainChunk {
  int model;
};

inline int get_tile_id(const int x, const int z) { return entt::hashed_string(TextFormat("tile_model_%d_%d", x, z)); }

export namespace terrain_streamer {
using TileCoord = std::pair<int, int>;

void stream(entt::registry& registry) {
  const auto& models = get_resource_manager(registry).models;
  const auto& offset = get_player(registry).offset;
  for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto entity : view) {
    const auto& [chunk, pos] = view.get<const TerrainChunk, const Position3D>(entity);
    const auto position = pos.pos + offset;
    DrawModel(models[chunk.model]->res, position, 1.0f, WHITE);
  }
}

void process_loaded_chunks(entt::registry& registry) {
  for (const auto view = registry.view<AsyncTileLoad>(); const auto entity : view) {
    auto& [texture_future, heightmap_future, x, z] = view.get<AsyncTileLoad>(entity);

    // zero wait check if the threads done
    const bool tex_ready = texture_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    const bool height_ready = heightmap_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;

    // if not, we'll try again next tick
    if (!tex_ready || !height_ready) continue;

    TraceLog(LOG_WARNING, "processing loaded tile %d %d", x, z);
    auto& rm = get_resource_manager(registry);
    const auto id = get_tile_id(x, z);

    const Image tex_img = texture_future.get();
    const Image height_img = heightmap_future.get();

    // create the texture
    const Texture2D final_texture = LoadTextureFromImage(tex_img);

    // keeping the texture in the resource manager
    // rm.textures.load(id, texture_future);

    // create the heightmap mesh and convert to model and apply texture
    const Mesh mesh = GenMeshHeightmap(height_img, (Vector3){TILE_WORLD_SIZE + TILE_OVERLAP, HIGHEST - LOWEST, TILE_WORLD_SIZE + TILE_OVERLAP});
    const Model model = LoadModelFromMesh(mesh);
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = final_texture;

    // if the fog exists in the resource manager, use it

    if (constexpr auto fog_id = entt::hashed_string("fog_shader"); rm.shaders.contains(fog_id)) {
      model.materials[0].shader = rm.shaders[fog_id]->res;
    }

    // keeping the tile in the resource manager
    rm.models.load(id, model);

    // unload images
    UnloadImage(tex_img);
    UnloadImage(height_img);

    registry.remove<AsyncTileLoad>(entity);
    registry.emplace<TerrainChunk>(entity, id);
    break; // to free the loop and let the next chunk load on the next frame
  }
}

class streamer {
  std::map<TileCoord, entt::entity> active_tiles;
  int last_tile_x = -9999;
  int last_tile_z = -9999;

 public:
  void update(entt::registry& registry) {
    const auto& player = get_player(registry);
    auto& rm = get_resource_manager(registry);

    // player absolute position
    const auto position = player.pos - player.offset;

    // the current tile based on position
    int current_tile_x = static_cast<int>(std::floor(position.x / TILE_WORLD_SIZE));
    int current_tile_z = static_cast<int>(std::floor(position.z / TILE_WORLD_SIZE));

    current_tile_x = std::clamp(current_tile_x, 0, X_TILES - 1);
    current_tile_z = std::clamp(current_tile_z, 0, Z_TILES - 1);

    // we are on the same tile as before, bye bye...
    if (current_tile_x == last_tile_x && current_tile_z == last_tile_z) return;

    // prepare list of required tiles
    std::set<TileCoord> required_tiles;
    for (int dx = -3; dx <= 3; ++dx) {
      for (int dz = -3; dz <= 3; ++dz) {
        auto required_x = current_tile_x + dx;
        auto required_z = current_tile_z + dz;
        if (required_x < 0 || required_x >= X_TILES || required_z < 0 || required_z >= Z_TILES) continue;
        required_tiles.insert({required_x, required_z});
      }
    }

    // iterating active and remove tiles not on required
    for (auto it = active_tiles.begin(); it != active_tiles.end();) {
      if (!required_tiles.contains(it->first)) {
        registry.destroy(it->second);
        // UnloadTexture(rm.models[get_tile_id(it->first.first, it->first.second)]->res.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture);
        const auto id = get_tile_id(it->first.first, it->first.second);
        // rm.textures.erase(id);
        rm.models.erase(id);
        it = active_tiles.erase(it);

      } else {
        ++it;
      }
    }

    // iterate the required tile and find not loaded
    for (const auto& coord : required_tiles) {
      if (!active_tiles.contains(coord)) {
        TraceLog(LOG_WARNING, "spawning tile: %d %d", coord.first, coord.second);
        const entt::entity new_tile_entity = spawn_tile(registry, coord.first, coord.second);
        active_tiles[coord] = new_tile_entity;
      }
    }

    last_tile_x = current_tile_x;
    last_tile_z = current_tile_z;
  }

 private:
  entt::entity spawn_tile(entt::registry& registry, const int x, const int z) {
    const auto entity = registry.create();

    std::string tex_path = TextFormat("assets/tiles/tex-%d-%d.png", x, z);
    std::string height_path = TextFormat("assets/tiles/hm-%d-%d.png", x, z);

    auto tex_task = std::async(std::launch::async, [tex_path]() {
      TraceLog(LOG_WARNING, "[thread] loading texture %s", tex_path.c_str());
      return LoadImage(tex_path.c_str());
    });

    auto height_task = std::async(std::launch::async, [height_path]() {
      TraceLog(LOG_WARNING, "[thread] loading height %s", height_path.c_str());
      Image height_img = LoadImage(height_path.c_str());
      ImageResize(&height_img, 256, 256);
      return height_img;
    });

    const float world_x = static_cast<float>(x) * TILE_WORLD_SIZE;
    const float world_z = static_cast<float>(z) * TILE_WORLD_SIZE;

    registry.emplace<Position3D>(entity, (Vector3){world_x, LOWEST, world_z}, Vector3Zero());
    registry.emplace<AsyncTileLoad>(entity, std::move(tex_task), std::move(height_task), x, z);

    return entity;
  }
};
}  // namespace terrain_streamer