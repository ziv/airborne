module;
#include <algorithm>
#include <entt/entt.hpp>
#include <format>
#include <future>
#include <map>
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
import TileDownloader;
import :Utils;

constexpr Meter UPDATE_THRESHOLD = 100.0f * 100.0f;  // player must move at least this much (squared) to trigger a terrain update
constexpr Meter SKIRT_SIZE = 0.0f;
// constexpr Meter TILE_SIZE_12 = 9783.9f;  // zoom 12
constexpr Meter TILE_SIZE_13 = 4891.95f;
constexpr Meter TILE_SIZE_14 = 2445.975f;
// constexpr int ZOOM_LEVEL = 12;
constexpr int BASE_X = 2444;
constexpr int BASE_Z = 1655;

// disc radius in z12 tile-units squared
constexpr int RENDER_DISC_R2 = 36;
constexpr Meter RENDER_RADIUS = 6 * TILE_SIZE_12;
constexpr Meter Z13_THRESHOLD = RENDER_RADIUS * 0.5f;
constexpr Meter Z14_THRESHOLD = RENDER_RADIUS * 0.25f;
constexpr Meter Z13_THRESHOLD_SQ = Z13_THRESHOLD * Z13_THRESHOLD;
constexpr Meter Z14_THRESHOLD_SQ = Z14_THRESHOLD * Z14_THRESHOLD;

// constexpr Meter tile_size_for_zoom(const int zoom) { return TILE_SIZE_12 / static_cast<Meter>(1 << (zoom - ZOOM_LEVEL)); }
//
// // world-space center for a tile at any zoom, aligned within its z12 parent area.
// inline float tile_world_pos(const int zoom, const int local_idx) {
//   const int n = 1 << (zoom - ZOOM_LEVEL);
//   const int parent = local_idx >> (zoom - ZOOM_LEVEL);
//   const int child = local_idx - parent * n;  // 0 .. n-1
//   const Meter sz = tile_size_for_zoom(zoom);
//   return static_cast<float>(parent) * TILE_SIZE_12 + (static_cast<float>(child) - (static_cast<float>(n) - 1.0f) * 0.5f) * sz;
// }

// Model create_model(const Meter size) { return LoadModelFromMesh(GenMeshPlane(size + size * 0.02f, size + size * 0.02f, 256, 256)); }

float tile_distance(const Vector3& player_pos, const int zoom, const int tx, const int tz) {
  const float tile_size = TILE_SIZE_12 / static_cast<float>(1 << (zoom - ZOOM_LEVEL));
  const float world_x = (static_cast<float>(tx) + 0.5f) * tile_size;
  const float world_z = (static_cast<float>(tz) + 0.5f) * tile_size;
  const float ddx = player_pos.x - world_x;
  const float ddz = player_pos.z - world_z;
  return ddx * ddx + ddz * ddz;
}

export struct AsyncTileLoad {
  std::shared_future<Image> texture_future;
  std::shared_future<Image> heightmap_future;
  int x = 0;
  int z = 0;
  int zoom = 12;
};

export struct AsyncTileLoadDebug {
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

int get_tex_id(const int zoom, const int x, const int z) { return entt::hashed_string(TextFormat("tile_tex_%d_%d_%d", zoom, x, z)); }
int get_height_id(const int zoom, const int x, const int z) { return entt::hashed_string(TextFormat("tile_height_%d_%d_%d", zoom, x, z)); }

void unload_tile(ResourceManager& rm, const int zoom, const int x, const int z) {
  rm.textures.erase(get_tex_id(zoom, x, z));
  rm.textures.erase(get_height_id(zoom, x, z));
  rm.images.erase(get_height_id(zoom, x, z));
}

export namespace terrain_streamer {

// returns the terrain elevation (metres, Mapbox RGB encoding) at a world-space
// XZ position by sampling the highest-resolution loaded tile that covers it.
// falls back through z14 -> z13 -> z12 so it works while tiles are still loading.
// @see https://docs.mapbox.com/data/tilesets/reference/mapbox-terrain-rgb-v1/#elevation-data
float ground_height_at(entt::registry& registry, const Vector3& pos) {
  // todo commented for debugging
  // auto& rm = get_resource_manager(registry);
  // for (const int zoom : {14, 13, 12}) {
  //   const float tile_size = TILE_SIZE_12 / static_cast<float>(1 << (zoom - ZOOM_LEVEL));
  //   const int tx = static_cast<int>(std::floor(pos.x / tile_size));
  //   const int tz = static_cast<int>(std::floor(pos.z / tile_size));
  //
  //   const int height_id = get_height_id(zoom, tx, tz);
  //   // NOLINTBEGIN
  //   if (!rm.images.contains(height_id)) continue;
  //
  //   const Image& img = rm.images[height_id]->res;
  //   const float u = pos.x / tile_size - static_cast<float>(tx);
  //   const float v = pos.z / tile_size - static_cast<float>(tz);
  //   const int px = std::clamp(static_cast<int>(u * static_cast<float>(img.width)), 0, img.width - 1);
  //   const int pz = std::clamp(static_cast<int>(v * static_cast<float>(img.height)), 0, img.height - 1);
  //
  //   const auto c = GetImageColor(img, px, pz);
  //   return -10000.0f + (static_cast<float>(c.r) * 65536.0f + static_cast<float>(c.g) * 256.0f + static_cast<float>(c.b)) * 0.1f;
  //   // NOLINTEND
  // }
  return 0.0f;  // no tile loaded yet
}

struct TileKey {
  int zoom;
  int x;
  int z;
  auto operator<=>(const TileKey&) const = default;
};

TileKey parent(const TileKey& key) { return TileKey{key.zoom - 1, key.x >> 1, key.z >> 1}; }

std::vector<TileKey> children(const TileKey& key) {
  const int child_zoom = key.zoom + 1;
  const int child_x = key.x << 1;
  const int child_z = key.z << 1;
  return {TileKey{child_zoom, child_x, child_z}, TileKey{child_zoom, child_x + 1, child_z}, TileKey{child_zoom, child_x, child_z + 1},
          TileKey{child_zoom, child_x + 1, child_z + 1}};
}

std::vector<TileKey> grand_children(const TileKey& key) {
  const int child_zoom = key.zoom + 2;
  const int child_x = key.x << 2;
  const int child_z = key.z << 2;
  std::vector<TileKey> result;
  for (int dx = 0; dx < 4; dx++) {
    for (int dz = 0; dz < 4; dz++) {
      result.push_back(TileKey{child_zoom, child_x + dx, child_z + dz});
    }
  }
  return result;
}

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
      : displacement_shader(LoadShader(resources::terrain_vertex_shader_path, resources::terrain_fragment_shader_path)),
        terrain_model12(std::make_unique<Model>(terrain_streamer_utils::create_model(TILE_SIZE_12))),
        terrain_model13(std::make_unique<Model>(terrain_streamer_utils::create_model(TILE_SIZE_13))),
        terrain_model14(std::make_unique<Model>(terrain_streamer_utils::create_model(TILE_SIZE_14))) {
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
      const Vector3 world_pos = pos.pos + player.offset + Vector3{0.0f, 200.0f, 0.0f};

      // "In front of camera" filter via dot product with forward
      const Vector3 to_tile = world_pos - camera.position;
      if (Vector3DotProduct(to_tile, player.forward) <= 0.0f) continue;

      const Vector2 sp = GetWorldToScreen(world_pos, camera);
      if (sp.x < 0.0f || sp.x > width || sp.y < 0.0f || sp.y > height) continue;

      DrawText(TextFormat("%d", chunk.zoom), static_cast<int>(sp.x), static_cast<int>(sp.y), 15, BLACK);
      DrawText(TextFormat("%d %d", chunk.x, chunk.z), static_cast<int>(sp.x) + 20, static_cast<int>(sp.y), 10, BLACK);
    }
  }

  void stream_debug(entt::registry& registry, const Camera3D& camera) const {
    const auto& player = get_player(registry);
    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      auto color = RED;
      auto size = TILE_SIZE_12;
      if (chunk.zoom == 13) {
        color = GREEN;
        size = TILE_SIZE_13;
      }
      if (chunk.zoom == 14) {
        color = BLUE;
        size = TILE_SIZE_14;
      }
      DrawCube(pos.pos + player.offset, size, 5.0f, size, color);
    }
  }

  void stream(entt::registry& registry, const Camera3D& camera) const {
    const auto& player = get_player(registry);
    const auto& rm = get_resource_manager(registry);

    SetShaderValue(displacement_shader, cam_pos_loc, &camera.position, SHADER_UNIFORM_VEC3);

    // we use BLEND_ALPHA to allow transparency in 3d (raylib default is not to allow)
    // we use transparency in shader to mimic fog
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
    // remove from rendered those that not needed every frame
    std::erase_if(rendered_tiles, [&](const auto& item) {
      const auto [key, entity] = item;
      if (desired_tiles.contains(key)) return false;
      if (!is_tile_covered(key) && !is_tile_out_of_range(key)) return false;

      // side effect (yes, I know...)
      const auto& [zoom, x, z] = key;
      // TraceLog(LOG_DEBUG, "unloading tile z%d %d %d", zoom, x, z);
      registry.destroy(entity);
      unload_tile(get_resource_manager(registry), zoom, x, z);
      return true;
    });

    // check for missing
    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      if (const auto key = TileKey{chunk.zoom, chunk.x, chunk.z}; !desired_tiles.contains(key) && !rendered_tiles.contains(key)) {
        registry.destroy(entity);
        unload_tile(get_resource_manager(registry), key.zoom, key.x, key.z);
        // TraceLog(LOG_WARNING, "tile z%d %d %d is rendered but not defined, removed", chunk.zoom, chunk.x, chunk.z);
      }
    }

    const auto player_pos = get_player(registry).absolute_position();

    if (Vector3DistanceSqr(player_pos, last_position) < UPDATE_THRESHOLD) return;  // only update when moved more than N meters
    last_position = player_pos;

    const int current_tile_x = static_cast<int>(std::floor(player_pos.x / TILE_SIZE_12));
    const int current_tile_z = static_cast<int>(std::floor(player_pos.z / TILE_SIZE_12));

    // build list or desired tiles keys
    std::vector<TileKey> new_desired_keys;
    new_desired_keys.reserve(512);

    // recursively subdivide a tile if it's within the threshold for the next zoom.
    // at max zoom (14) always add the leaf. otherwise, check the tile center distance.
    auto subdivide = [&](auto& self, const int zoom, const int tx, const int tz) -> void {
      const float dist_sq = tile_distance(player_pos, zoom, tx, tz);

      if (zoom == 14 || (zoom == 13 && dist_sq >= Z14_THRESHOLD_SQ) || (zoom == 12 && dist_sq >= Z13_THRESHOLD_SQ)) {
        new_desired_keys.push_back({zoom, tx, tz});
        return;
      }

      // split into 4 children at zoom+1.
      const int child_zoom = zoom + 1;
      const int cx0 = tx * 2;
      const int cz0 = tz * 2;
      for (int ox = 0; ox < 2; ++ox)
        for (int oz = 0; oz < 2; ++oz) self(self, child_zoom, cx0 + ox, cz0 + oz);
    };

    for (int dx = -7; dx <= 7; ++dx) {
      for (int dz = -7; dz <= 7; ++dz) {
        if (dz * dz + dx * dx > RENDER_DISC_R2) continue;
        subdivide(subdivide, 12, current_tile_x + dx, current_tile_z + dz);
      }
    }

    // sort for searching (still cheaper than set? YES, set required the heap and O(n log n) for searching)
    std::ranges::sort(new_desired_keys);

    // remove from desired those that not in new desired
    std::erase_if(desired_tiles, [&](const auto& item) {
      const auto [key, entity] = item;
      const auto remove = !std::ranges::binary_search(new_desired_keys, key);

      // side effect (yes, I know...)
      if (remove && registry.all_of<AsyncTileLoadDebug>(entity)) registry.destroy(entity);
      return remove;
    });

    // remove from rendered those that not needed
    // std::erase_if(rendered_tiles, [&](const auto& item) {
    //   const auto [key, entity] = item;
    //   if (desired_tiles.contains(key)) return false;
    //   if (!is_tile_covered(key) && !is_tile_out_of_range(key)) return false;
    //
    //   // side effect (yes, I know...)
    //   const auto& [zoom, x, z] = key;
    //   registry.destroy(entity);
    //   unload_tile(get_resource_manager(registry), zoom, x, z);
    //   return true;
    // });

    // spawn new
    for (const auto& key : new_desired_keys) {
      if (!desired_tiles.contains(key)) desired_tiles[key] = spawn_tile(registry, key);
    }
  }

  void process_loaded_chunks(entt::registry& registry) {
    for (const auto view = registry.view<AsyncTileLoadDebug>(); const auto [entity, tile] : view.each()) {
      registry.remove<AsyncTileLoadDebug>(entity);
      registry.emplace<TerrainChunk>(entity, 0, 0, tile.zoom, tile.x, tile.z);
      registry.emplace<TerrainHeight>(entity, 0);
      rendered_tiles[{tile.zoom, tile.x, tile.z}] = entity;
      break;
    }
    // todo commented for debugging
    // for (const auto view = registry.view<AsyncTileLoad>(); const auto [entity, tile] : view.each()) {
    //   const bool tex_ready = tile.texture_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    //   const bool height_ready = tile.heightmap_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    //
    //   if (!tex_ready || !height_ready) continue;
    //
    //   Image tex_img = tile.texture_future.get();
    //   Image height_img = tile.heightmap_future.get();
    //
    //   const Texture2D texture_tex = LoadTextureFromImage(tex_img);
    //   const Texture2D height_tex = LoadTextureFromImage(height_img);
    //
    //   UnloadImage(tex_img);
    //
    //   const auto texture_id = get_tex_id(tile.zoom, tile.x, tile.z);
    //   const auto height_id = get_height_id(tile.zoom, tile.x, tile.z);
    //
    //   auto& rm = get_resource_manager(registry);
    //   unload_tile(rm, tile.zoom, tile.x, tile.z);
    //
    //   rm.textures.load(texture_id, texture_tex);
    //   rm.textures.load(height_id, height_tex);
    //   rm.images.load(height_id, height_img);
    //
    //   registry.remove<AsyncTileLoad>(entity);
    //   registry.emplace<TerrainChunk>(entity, texture_id, height_id, tile.zoom, tile.x, tile.z);
    //   registry.emplace<TerrainHeight>(entity, height_id);
    //
    //   rendered_tiles[{tile.zoom, tile.x, tile.z}] = entity;
    //   break;
    // }
  }

 private:
  static entt::entity spawn_tile(entt::registry& registry, const TileKey& tile) {
    const auto entity = registry.create();

    const int scale = 1 << (tile.zoom - ZOOM_LEVEL);
    const int tx = tile.x + BASE_X * scale;
    const int tz = tile.z + BASE_Z * scale;
    TraceLog(LOG_DEBUG, "spawning tile z%d %d %d %d %d", tile.zoom, tile.x, tile.z, tx, tz);

    // todo commented for debugging
    std::string tex_path = std::format("assets/tiles/texture/{}/{}/{}.png", tile.zoom, tx, tz);
    std::string height_path = std::format("assets/tiles/heightmaps/{}/{}/{}.png", tile.zoom, tx, tz);

    const auto mapbox_token = std::string(std::getenv("MAPBOX_TOKEN") ? std::getenv("MAPBOX_TOKEN") : "");
    const std::string tex_url = tile_downloader::texture_url(tile.zoom, tx, tz, mapbox_token);
    const std::string height_url = tile_downloader::heightmap_url(tile.zoom, tx, tz, mapbox_token);

    auto tex_future = tile_downloader::enqueue_and_load(tex_path, tex_url);
    auto height_future = tile_downloader::enqueue_and_load(height_path, height_url);

    const float tile_size = TILE_SIZE_12 / static_cast<float>(1 << (tile.zoom - ZOOM_LEVEL));
    const float world_x = (static_cast<float>(tile.x) + 0.5f) * tile_size;
    const float world_z = (static_cast<float>(tile.z) + 0.5f) * tile_size;

    registry.emplace<Position3D>(entity, (Vector3){world_x, 0.0f, world_z});
    registry.emplace<AsyncTileLoadDebug>(entity, tile.x, tile.z, tile.zoom);
    // registry.emplace<AsyncTileLoad>(entity, std::move(tex_future), std::move(height_future), tile.x, tile.z, tile.zoom);

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

  // todo NAIVE implementation by purpose, need a performance refactoring
  [[nodiscard]] bool is_tile_out_of_range(const TileKey& key) const {
    const float tile_size = TILE_SIZE_12 / static_cast<float>(1 << (key.zoom - ZOOM_LEVEL));
    const float world_x = (static_cast<float>(key.x) + 0.5f) * tile_size;
    const float world_z = (static_cast<float>(key.z) + 0.5f) * tile_size;
    const float ddx = last_position.x - world_x;
    const float ddz = last_position.z - world_z;
    const float dist_sq = ddx * ddx + ddz * ddz;
    return dist_sq > RENDER_RADIUS * RENDER_RADIUS;
  }

  // todo NAIVE implementation by purpose, need a performance refactoring
  [[nodiscard]] bool is_tile_covered(const TileKey& key) const {
    // if it is zoom 14, we need to check there is a prent exists
    if (key.zoom == 14) {
      const auto p = parent(key);
      if (rendered_tiles.contains(p)) return true;
      if (rendered_tiles.contains(parent(p))) return true;
      return false;
    }
    // if it is zoom 13 we need to check parent and children
    if (key.zoom == 13) {
      if (rendered_tiles.contains(parent(key))) return true;
      return std::ranges::all_of(children(key), [&](const TileKey& x) { return rendered_tiles.contains(x); });
    }
    // if it is zoom 12 we need to check children
    if (key.zoom == 12) {
      if (std::ranges::all_of(children(key), [&](const TileKey& x) { return rendered_tiles.contains(x); })) return true;
      return std::ranges::all_of(grand_children(key), [&](const TileKey& x) { return rendered_tiles.contains(x); });
    }
    return false;
  }
};
}  // namespace terrain_streamer