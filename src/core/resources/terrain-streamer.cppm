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
import Types;

// inline constexpr int MIN_X = 2444;
// inline constexpr int MIN_Z = 1644;

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

class streamer {
  // TilesDef& tiles;
  std::map<TileCoord, entt::entity> active_tiles;
  int last_tile_x = -9999;
  int last_tile_z = -9999;

 public:
  // explicit streamer(TilesDef& tls) : tiles(tls) {}

  void update(entt::registry& registry) {
    const auto& player = get_player(registry);
    const TilesDef& scenario = get_tiles_def(registry);
    auto& rm = get_resource_manager(registry);

    // player absolute position
    const auto position = player.pos - player.offset;

    // the current tile based on position
    const auto tile_world_size = scenario.meter_to_pixel * scenario.tex_size;
    int current_tile_x = static_cast<int>(std::floor(position.x / tile_world_size));
    int current_tile_z = static_cast<int>(std::floor(position.z / tile_world_size));

    current_tile_x = std::clamp(current_tile_x, 0, scenario.x_count - 1);
    current_tile_z = std::clamp(current_tile_z, 0, scenario.z_count - 1);

    // we are on the same tile as before, bye bye...
    // but before we return, we need to measure the height below us...
    if (current_tile_x == last_tile_x && current_tile_z == last_tile_z) return;

    // prepare list of required tiles
    std::set<TileCoord> required_tiles;
    for (int dx = -3; dx <= 3; ++dx) {
      for (int dz = -3; dz <= 3; ++dz) {
        auto required_x = current_tile_x + dx;
        auto required_z = current_tile_z + dz;
        if (required_x < 0 || required_x >= scenario.x_count || required_z < 0 || required_z >= scenario.z_count) continue;
        required_tiles.insert({required_x, required_z});
      }
    }

    // iterating active and remove tiles not on required
    for (auto it = active_tiles.begin(); it != active_tiles.end();) {
      if (!required_tiles.contains(it->first)) {
        registry.destroy(it->second);
        // UnloadTexture(rm.models[get_tile_id(it->first.first, it->first.second)]->res.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture);
        const auto id = get_tile_id(it->first.first, it->first.second);
        rm.models.erase(id);
        rm.textures.erase(id);
        it = active_tiles.erase(it);

      } else {
        ++it;
      }
    }

    // iterate the required tile and find not loaded
    for (const auto& coord : required_tiles) {
      if (!active_tiles.contains(coord)) {
        TraceLog(LOG_WARNING, "spawning tile: %d %d", coord.first, coord.second);
        const entt::entity new_tile_entity = spawn_tile(registry, coord.first, coord.second, scenario);
        active_tiles[coord] = new_tile_entity;
      }
    }

    last_tile_x = current_tile_x;
    last_tile_z = current_tile_z;
  }

  void process_loaded_chunks(entt::registry& registry) {
    for (const auto view = registry.view<AsyncTileLoad>(); const auto entity : view) {
      auto& [texture_future, heightmap_future, x, z] = view.get<AsyncTileLoad>(entity);
      const TilesDef& scenario = get_tiles_def(registry);

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

      // create the heightmap mesh and convert to model and apply texture
      const auto tile_world_size = scenario.meter_to_pixel * scenario.tex_size;
      const auto tile_overlap = tile_world_size / 255.0f;
      const Mesh mesh =
          GenMeshHeightmap(height_img, (Vector3){tile_world_size + tile_overlap, scenario.highest - scenario.lowest, tile_world_size + tile_overlap});
      const Model model = LoadModelFromMesh(mesh);
      model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = final_texture;

      // if the fog exists in the resource manager, use it

      // if (constexpr auto fog_id = entt::hashed_string("fog_shader"); rm.shaders.contains(fog_id)) {
      // model.materials[0].shader = rm.shaders[fog_id]->res;
      // }

      // keeping the tile in the resource manager
      rm.models.load(id, model);
      rm.textures.load(id, final_texture);

      // unload images
      UnloadImage(tex_img);
      UnloadImage(height_img);

      registry.remove<AsyncTileLoad>(entity);
      registry.emplace<TerrainChunk>(entity, id);
      break;  // to free the loop and let the next chunk load on the next frame
    }
  }

 private:
  entt::entity spawn_tile(entt::registry& registry, const int x, const int z, const TilesDef& tiles) {
    const auto entity = registry.create();

    std::string tex_path = TextFormat(tiles.tex_path.c_str(), x + tiles.min_x, z + tiles.min_z);
    std::string height_path = TextFormat(tiles.hmp_path.c_str(), x + tiles.min_x, z + tiles.min_z);

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

    const auto tile_world_size = tiles.meter_to_pixel * tiles.tex_size;
    const float world_x = static_cast<float>(x) * tile_world_size;
    const float world_z = static_cast<float>(z) * tile_world_size;

    registry.emplace<Position3D>(entity, (Vector3){world_x, tiles.lowest, world_z}, Vector3Zero());
    registry.emplace<AsyncTileLoad>(entity, std::move(tex_task), std::move(height_task), x, z);

    return entity;
  }
};
}  // namespace terrain_streamer