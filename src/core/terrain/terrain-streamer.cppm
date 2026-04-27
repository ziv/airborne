module;
// #define CPPHTTPLIB_OPENSSL_SUPPORT
#include <entt/entt.hpp>
#include <future>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <tuple>
#include <iostream>

// #include "../../lib/httplib.h"
#include "../../lib/ray.hpp"

export module TerrainStreaming;

import Components;
import RaylibResource;
import ResourceManager;
import Resources;
import Accessors;
import Types;

const auto mapbox_url = std::string("https://api.mapbox.com");
const auto mapbox_heightmap = std::string("/v4/mapbox.terrain-rgb/{}/{}/{}.pngraw?access_token={}");
const auto mapbox_texture = std::string("/v4/mapbox.satellite/{}/{}/{}.png?access_token={}");

const auto cache_heightmap = std::string("assets/tiles/cache/heightmaps/{}/{}/{}.png");
const auto cache_heightmap_dir = std::string("assets/tiles/cache/heightmaps/{}/{}");
const auto cache_texture = std::string("assets/tiles/cache/texture/{}/{}/{}.png");
const auto cache_texture_dir = std::string("assets/tiles/cache/texture/{}/{}");

constexpr auto ZOOM = 14;
constexpr auto RADIUS = 3;
constexpr auto TILE_SIZE = 2445.985f;  // zoom 14
constexpr auto ANCHORE_TILE_X = 9774;  // todo anchore should be at zoom 10
constexpr auto ANCHORE_TILE_Z = 6646;

struct Vector2D {
  double x, y;
};

struct TileID {
  int z, x, y;

  bool operator==(const TileID& other) const { return z == other.z && x == other.x && y == other.y; }
};

constexpr double EARTH_CIRCUMFERENCE = 40075016.686;

inline double get_tile_size_meters(const int zoom) {
  const auto tilesPerSide = static_cast<double>(1ULL << zoom);
  return EARTH_CIRCUMFERENCE / tilesPerSide;
}

inline std::string heightmap_url(const int zoom, const int x, const int z, const std::string& token) {
  return std::vformat(mapbox_heightmap, std::make_format_args(zoom, x, z, token));
}

inline std::string texture_url(const int zoom, const int x, const int z, const std::string& token) {
  return std::vformat(mapbox_texture, std::make_format_args(zoom, x, z, token));
}

inline std::string heightmap_path(const int zoom, const int x, const int z) { return std::vformat(cache_heightmap_dir, std::make_format_args(zoom, x, z)); }

inline std::string texture_path(const int zoom, const int x, const int z) { return std::vformat(cache_texture_dir, std::make_format_args(zoom, x, z)); }

inline std::string heightmap_file(const int zoom, const int x, const int z) { return std::vformat(cache_heightmap, std::make_format_args(zoom, x, z)); }

inline std::string texture_file(const int zoom, const int x, const int z) { return std::vformat(cache_texture, std::make_format_args(zoom, x, z)); }

// inline Vector2D lat_lon_to_mercator_meters(const double lat, const double lon) {
//   Vector2D meters{};
//   const double lat_rad = lat * PI / 180.0;
//   meters.y = std::log(std::tan((PI / 4.0) + (lat_rad / 2.0))) * (EARTH_CIRCUMFERENCE / (2.0 * PI));
//   meters.x = lon * EARTH_CIRCUMFERENCE / 360.0;
//   return meters;
// }

// inline TileID get_tile_from_world_pos(double world_x, double world_z, int zoom) {
//   Vector2D anchor_mercator = lat_lon_to_mercator_meters(ANCHOR_LAT, ANCHOR_LON);
//   // adding the anchor
//   double global_merc_x = world_x + anchor_mercator.x;
//   double global_merc_y = (-world_z) + anchor_mercator.y;
//
//   double map_size_meters = EARTH_CIRCUMFERENCE;
//   double tiles_per_side = std::pow(2, zoom);
//   double tile_size_meters = map_size_meters / tiles_per_side;
//
//   double normalized_x = (global_merc_x + (map_size_meters / 2.0)) / map_size_meters;
//   double normalized_y = ((map_size_meters / 2.0) - global_merc_y) / map_size_meters;
//
//   TileID tile{};
//   tile.z = zoom;
//   tile.x = std::floor(normalized_x * tiles_per_side);
//   tile.y = std::floor(normalized_y * tiles_per_side);
//
//   return tile;
// }

// std::tuple<bool, std::string> download_file(const std::string& path, int zoom, int x, int z) {
//   const char* envToken = std::getenv("MAPBOX_TOKEN");
//
//   if (envToken == nullptr) {
//     std::cerr << "Error: MAPBOX_TOKEN environment variable is not set!\n";
//     return {false, ""};
//   }
//
//   const auto token = std::string(envToken);
//   httplib::Client cli("https://api.mapbox.com");
//   const auto request = std::vformat(path, std::make_format_args(zoom, x, z, token));
//   auto res = cli.Get(request);
//
//   if (res && res->status == 200) return {true, res->body};
//
//   return {false, ""};
// }

export struct AsyncTileLoad {
  std::future<Image> texture_future;
  std::future<Image> heightmap_future;
  int x = 0;
  int z = 0;
};

export struct TerrainChunk {
  int model;
  int height;
  int x = 0;
  int z = 0;
};

inline int get_tile_id(const int x, const int z) { return entt::hashed_string(TextFormat("tile_model_%d_%d", x, z)); }
inline int get_tex_id(const int x, const int z) { return entt::hashed_string(TextFormat("tile_tex_%d_%d", x, z)); }
inline int get_height_id(const int x, const int z) { return entt::hashed_string(TextFormat("tile_height_%d_%d", x, z)); }

export namespace terrain_streamer {
std::string download_file(const std::string& mapbox_url, const std::string& request_path);

using TileCoord = std::pair<int, int>;

void stream(entt::registry& registry, const Camera3D& camera) {
  const auto& models = get_resource_manager(registry).models;
  const auto& offset = get_player(registry).offset;

  if (-99 != resources::fog_shader_pos_loc) {
    const auto& shader = get_resource_manager(registry).shaders[resources::fog_shader]->res;
    SetShaderValue(shader, resources::fog_shader_pos_loc, &camera.position, SHADER_UNIFORM_VEC3);
  }

  for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto entity : view) {
    const auto& [chunk, pos] = view.get<const TerrainChunk, const Position3D>(entity);
    const Vector3 position = pos.pos + offset;

    DrawModel(models[chunk.model]->res, position, 1.0f, WHITE);
  }
}

/// @brief generate flat plane model
Model create_model() {
  Mesh planeMesh = GenMeshPlane(TILE_SIZE, TILE_SIZE, 256, 256);
  return LoadModelFromMesh(planeMesh);
}

Image get_image(const std::string& tex_path, const std::string& request_path, const int zoom, const int tx, const int ty) {
  const auto file_path = std::vformat(tex_path, std::make_format_args(zoom, tx, ty));

  // exists in cache, use it
  if (std::filesystem::exists(file_path)) {
    std::cout << "Texture tile already exists in cache, skipping download.\n";
    return LoadImage(file_path.c_str());
  }

  std::cout << "Downloading tile" << std::to_string(tx) << "," << std::to_string(ty) << "\n";

  // ensure directory exists
  const std::string dir_path = std::filesystem::path(file_path).parent_path().string();
  std::filesystem::create_directories(dir_path);

  // get token to download tile
  const char* env_token = std::getenv("MAPBOX_TOKEN");
  if (env_token == nullptr) {
    std::cerr << "Error: MAPBOX_TOKEN environment variable is not set!\n";
    return LoadImage("path to default image?");
  }
  const auto token = std::string(env_token);

  // download tile
  const auto request = std::vformat(request_path, std::make_format_args(zoom, tx, ty, token));
  const auto body = download_file(mapbox_url, request);
  if (body.empty()) {
    std::cout << "Unable to download tile.\n";
    return LoadImage("path to default image?");
  }

  // save tile
  std::ofstream out_file(file_path, std::ios::binary);
  if (!out_file) {
    std::cerr << "Could not open file for writing: " << tex_path << "\n";
    return LoadImage("path-to-default-image?");
  }
  out_file.write(body.data(), static_cast<long>(body.size()));

  return LoadImage(file_path.c_str());
}

class streamer {
  // TilesDef& tiles;
  std::map<TileCoord, entt::entity> active_tiles;
  int last_tile_x = -9999;
  int last_tile_z = -9999;
  Shader displacement_shader;
  Model terrain_model;
  ResourceManager& resource_manager;
  // Vector3& offset;

 public:
  explicit streamer(entt::registry& reg)
      : displacement_shader(LoadShader("assets/shaders/terrain.vs", "assets/shaders/terrain.fs")),
        terrain_model(create_model()),
        resource_manager(get_resource_manager(reg)) {
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
  }

  void stream(entt::registry& registry, const Camera3D& camera) {
    // todo fog shader...?!
    const auto offset = get_player(registry).offset;
    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      // the texture is not exists (not suppose to happen, just for safety)
      if (!resource_manager.textures.contains(chunk.model)) continue;
      // the heightmap is not exists (not suppose to happen, just for safety)
      if (!resource_manager.textures.contains(chunk.height)) continue;

      // now this access is safe
      const auto tex = resource_manager.textures[chunk.model]->res;
      const auto heightmap = resource_manager.textures[chunk.height]->res;

      // attach the texture and the heightmap to the slot we defined in the ctr
      terrain_model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = tex;
      terrain_model.materials[0].maps[MATERIAL_MAP_ROUGHNESS].texture = heightmap;

      const Vector3 position = pos.pos + offset;
      DrawModel(terrain_model, position, 1.0f, WHITE);
    }
  }

  void update(entt::registry& registry) {
    // const auto& player = get_player(registry);
    const TilesDef& tiles = get_tiles_def(registry);
    auto& rm = get_resource_manager(registry);

    // player absolute position
    const auto position = get_player(registry).absolute_position();

    // the current tile based on position (relative tiles position)
    const auto tile_world_size = get_tile_size_meters(ZOOM);
    const int current_tile_x = static_cast<int>(std::floor(position.x / tile_world_size));
    const int current_tile_z = static_cast<int>(std::floor(position.z / tile_world_size));

    // we are on the same tile as before, bye bye...
    if (current_tile_x == last_tile_x && current_tile_z == last_tile_z) return;

    constexpr auto radius = RADIUS;

    // prepare list of required tiles
    std::set<TileCoord> required_tiles;
    for (int dx = -radius; dx <= radius; ++dx) {
      for (int dz = -radius; dz <= radius; ++dz) {
        auto required_x = current_tile_x + dx;
        auto required_z = current_tile_z + dz;
        required_tiles.insert({required_x, required_z});
      }
    }

    // iterating active and remove tiles not on required
    for (auto it = active_tiles.begin(); it != active_tiles.end();) {
      if (!required_tiles.contains(it->first)) {
        // unloading the texture from the not required tile
        const auto rx = it->first.first;
        const auto rz = it->first.second;
        rm.textures.erase(get_tex_id(rx, rz));
        rm.textures.erase(get_height_id(rx, rz));
        // remove the entity
        registry.destroy(it->second);
        it = active_tiles.erase(it);
      } else {
        ++it;
      }
    }

    // iterate the required tile and spawn not loaded ones
    for (const auto& coord : required_tiles) {
      if (!active_tiles.contains(coord)) {
        const entt::entity new_tile_entity = spawn_tile(registry, coord.first, coord.second, tiles);
        active_tiles[coord] = new_tile_entity;
      }
    }

    last_tile_x = current_tile_x;
    last_tile_z = current_tile_z;
  }

  void process_loaded_chunks(entt::registry& registry) {
    auto& rm = get_resource_manager(registry);
    const auto view = registry.view<AsyncTileLoad>();

    for (const auto [entity, atl] : view.each()) {
      // zero wait check if the threads done
      const bool tex_ready = atl.texture_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
      const bool height_ready = atl.heightmap_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;

      // if not, we'll try again next tick
      if (!tex_ready || !height_ready) continue;

      const Image tex_img = atl.texture_future.get();
      const Image height_img = atl.heightmap_future.get();

      const Texture2D texture_tex = LoadTextureFromImage(tex_img);
      const Texture2D height_tex = LoadTextureFromImage(height_img);

      // keep textures in resource manager to use them in the rendering phase
      const auto tex_id = get_tex_id(atl.x, atl.z);
      const auto height_id = get_height_id(atl.x, atl.z);
      rm.textures.load(tex_id, texture_tex);
      rm.textures.load(height_id, height_tex);

      // we've done with the images
      UnloadImage(height_img);
      UnloadImage(tex_img);

      // remove the component that cause this handler to run and attach the resources ids to the next one
      registry.remove<AsyncTileLoad>(entity);
      registry.emplace<TerrainChunk>(entity, tex_id, height_id, atl.x, atl.z);
      break;  // to free the loop and let the next chunk load on the next frame
    }
  }

 private:
  entt::entity spawn_tile(entt::registry& registry, const int x, const int z, const TilesDef& tiles) {
    const auto entity = registry.create();

    // the real tile coordinates (with the offset from the zero tile)
    const int tx = x + tiles.min_x;
    const int ty = z + tiles.min_z;
    constexpr auto zoom = ZOOM;

    TraceLog(LOG_WARNING, "spawning tile %d (%d), %d (%d)", x, tx, z, ty);

    // spawn 2 threads to load texture and heightmap in parallel

    auto tex_task = std::async(std::launch::async, [zoom, tx, ty]() {
      // TraceLog(LOG_WARNING, "[thread] loading height %s", height_path.c_str());
      return get_image(cache_texture, mapbox_texture, zoom, tx, ty);
    });

    auto height_task = std::async(std::launch::async, [zoom, tx, ty]() {
      // TraceLog(LOG_WARNING, "[thread] loading height %s", height_path.c_str());
      return get_image(cache_heightmap, mapbox_heightmap, zoom, tx, ty);
    });

    // the tiles position in the real world
    const auto tile_world_size = tiles.meter_to_pixel * tiles.tex_size;
    const float world_x = static_cast<float>(x) * tile_world_size;
    const float world_z = static_cast<float>(z) * tile_world_size;

    // add the futures to the entity component so the next system will handle the threads (process_loaded_chunks)
    registry.emplace<Position3D>(entity, (Vector3){world_x, 0.0f, world_z}, Vector3Zero());
    registry.emplace<AsyncTileLoad>(entity, std::move(tex_task), std::move(height_task), x, z);

    return entity;
  }
};
}  // namespace terrain_streamer