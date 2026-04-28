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
constexpr Meter TILE_SIZE = 9783.9;     // zoom 12
constexpr Meter TILE_SIZE_12 = 9783.9;  // zoom 12
constexpr int ZOOM_LEVEL = 12;
constexpr int BASE_X = 2444;
constexpr int BASE_Z = 1655;

// constexpr Meter TILE_SIZE = 2445.975f;  // zoom 14
// constexpr int BASE_X = 9755;
// constexpr int BASE_Z = 6627;

Model create_model() { return LoadModelFromMesh(GenMeshPlane(TILE_SIZE + SKIRT_SIZE, TILE_SIZE + SKIRT_SIZE, 256, 256)); }

export struct AsyncTileLoad {
  std::future<Image> texture_future;
  std::future<Image> heightmap_future;
  int x = 0;
  int z = 0;
};

export struct TerrainChunk {
  int model;
  int height;
};

export struct TerrainHeight {
  int height;  // id of the height model
};

inline int get_tile_id(const int x, const int z) { return entt::hashed_string(TextFormat("tile_model_%d_%d", x, z)); }
inline int get_tex_id(const int x, const int z) { return entt::hashed_string(TextFormat("tile_tex_%d_%d", x, z)); }
inline int get_height_id(const int x, const int z) { return entt::hashed_string(TextFormat("tile_height_%d_%d", x, z)); }

export namespace terrain_streamer {

struct TileKey {
  int zoom;
  int x;
  int z;
  auto operator<=>(const TileKey&) const = default;
};

void on_terrain_destroyed(entt::registry& reg, entt::entity entity) {
  TraceLog(LOG_INFO, "Destroying terrain chunk entity %d", static_cast<int>(entity));
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
  Model terrain_model;

 public:
  explicit streamer(entt::registry& registry)
      : displacement_shader(LoadShader("assets/shaders/terrain.vs", "assets/shaders/terrain.fs")), terrain_model(create_model()) {
    // set the displacement_shader as the terrain model shader
    terrain_model.materials[0].shader = displacement_shader;

    // set the heightmap data into MATERIAL_MAP_ROUGHNESS slot
    constexpr int heightmapSlotIndex = MATERIAL_MAP_ROUGHNESS;  // Raylib map roughness index
    const int shaderLocation = GetShaderLocation(displacement_shader, "heightMap");
    SetShaderValue(displacement_shader, shaderLocation, &heightmapSlotIndex, SHADER_UNIFORM_INT);

    // set scale 1 as long as the model is stretched properly
    constexpr float heightScale = 1.0;
    const int scaleLoc = GetShaderLocation(displacement_shader, "heightScale");
    SetShaderValue(displacement_shader, scaleLoc, &heightScale, SHADER_UNIFORM_FLOAT);

    // clean resources when destroying
    // registry.on_destroy<TerrainChunk>().connect<&on_terrain_destroyed>();
  }

  void stream(entt::registry& registry, const Camera3D& camera) {
    const auto& models = get_resource_manager(registry).models;
    const auto& offset = get_player(registry).offset;
    const auto& rm = get_resource_manager(registry);

    // todo fog...

    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      // the texture is not exists (not suppose to happen, just for safety)
      if (!rm.textures.contains(chunk.model)) continue;
      // the heightmap is not exists (not suppose to happen, just for safety)
      if (!rm.textures.contains(chunk.height)) continue;

      // now this access is safe
      const auto tex = rm.textures[chunk.model]->res;
      const auto heightmap = rm.textures[chunk.height]->res;

      // attach the texture and the heightmap to the slot we defined in the ctr
      terrain_model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = tex;
      terrain_model.materials[0].maps[MATERIAL_MAP_ROUGHNESS].texture = heightmap;

      const Vector3 position = pos.pos + offset;
      DrawModel(terrain_model, position, 1.0f, WHITE);
    }
  }

  void update(entt::registry& registry) {
    const auto& player = get_player(registry);
    // const TilesDef& tiles = get_tiles_def(registry);
    auto& rm = get_resource_manager(registry);

    // player absolute position
    const auto position = player.absolute_position();

    // the current tile based on position
    // const auto tile_world_size = tiles.meter_to_pixel * tiles.tex_size;
    const int current_tile_x = static_cast<int>(std::floor(position.x / TILE_SIZE));
    const int current_tile_z = static_cast<int>(std::floor(position.z / TILE_SIZE));

    // we are on the same tile as before, bye bye...
    if (current_tile_x == last_tile_x && current_tile_z == last_tile_z) return;

    // prepare list of required tiles
    std::set<TileKey> required_tiles;
    for (int dx = -6; dx <= 6; ++dx) {
      for (int dz = -6; dz <= 6; ++dz) {
        if (dz * dz + dx * dx > 30) continue;
        const auto bx = current_tile_x + dx;
        const auto bz = current_tile_z + dz;
        required_tiles.insert({12, bx, bz});
      }
    }

    // iterating active and remove tiles not on required
    for (auto it = active_tiles.begin(); it != active_tiles.end();) {
      if (!required_tiles.contains(it->first)) {
        // remove the entity of the tile
        registry.destroy(it->second);
        // clean textures of removed tiles
        const auto texture_id = get_tex_id(it->first.x, it->first.z);
        const auto height_id = get_height_id(it->first.x, it->first.z);
        rm.textures.erase(texture_id);
        rm.textures.erase(height_id);
        rm.images.erase(height_id);
        it = active_tiles.erase(it);
      } else {
        ++it;
      }
    }

    // iterate the required tile and find not loaded
    for (const auto& coord : required_tiles) {
      if (!active_tiles.contains(coord)) {
        // TraceLog(LOG_WARNING, "spawning tile: %d %d", coord.first, coord.second);
        const entt::entity new_tile_entity = spawn_tile(registry, coord.x, coord.z);
        active_tiles[coord] = new_tile_entity;
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

      const Image tex_img = tile.texture_future.get();
      const Image height_img = tile.heightmap_future.get();

      // create the texture
      const Texture2D texture_tex = LoadTextureFromImage(tex_img);
      const Texture2D height_tex = LoadTextureFromImage(height_img);

      // need no more
      UnloadImage(tex_img);

      // keep textures in resource manager for the render phase
      const auto texture_id = get_tex_id(tile.x, tile.z);
      const auto height_id = get_height_id(tile.x, tile.z);

      auto& rm = get_resource_manager(registry);
      rm.textures.load(texture_id, texture_tex);
      rm.textures.load(height_id, height_tex);
      rm.images.load(height_id, height_img);

      registry.remove<AsyncTileLoad>(entity);
      registry.emplace<TerrainChunk>(entity, texture_id, height_id);
      registry.emplace<TerrainHeight>(entity, height_id);
      break;  // to free the loop and let the next chunk load on the next frame
    }
  }

 private:
  entt::entity spawn_tile(entt::registry& registry, const int x, const int z) {
    const auto entity = registry.create();

    const int tx = x + BASE_X;
    const int ty = z + BASE_Z;
    TraceLog(LOG_WARNING, "spawning tile %d (%d), %d (%d)", x, tx, z, ty);

    std::string tex_path = std::format("assets/tiles/cache/texture/12/{}/{}.png", tx, ty);
    std::string height_path = std::format("assets/tiles/cache/heightmaps/12/{}/{}.png", tx, ty);

    auto ensure_tile = [](int zoom, int tx, int tz, const std::string& path) {
      if (!std::filesystem::exists(path)) {
        std::string cmd = "./download_tile.mjs " + std::to_string(zoom) + " " + std::to_string(tx) + " " + std::to_string(tz);
        std::system(cmd.c_str());
      }
      return LoadImage(path.c_str());
    };

    auto tex_task = std::async(std::launch::async, [ensure_tile, tx, ty, tex_path]() { return ensure_tile(ZOOM_LEVEL, tx, ty, tex_path); });

    auto height_task = std::async(std::launch::async, [ensure_tile, tx, ty, height_path]() { return ensure_tile(ZOOM_LEVEL, tx, ty, height_path); });

    const float world_x = static_cast<float>(x) * TILE_SIZE;
    const float world_z = static_cast<float>(z) * TILE_SIZE;

    registry.emplace<Position3D>(entity, (Vector3){world_x, 0.0f, world_z}, Vector3Zero());
    registry.emplace<AsyncTileLoad>(entity, std::move(tex_task), std::move(height_task), x, z);

    return entity;
  }
};
}  // namespace terrain_streamer