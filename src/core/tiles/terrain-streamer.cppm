module;
#include <algorithm>
#include <entt/entt.hpp>
#include <format>
#include <future>
#include <string>
#include <unordered_map>
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

// components

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

export namespace terrain_streamer {

/// returns the terrain elevation (metres, Mapbox RGB encoding) at a world-space
/// XZ position by sampling the highest-resolution loaded tile that covers it.
/// falls back through z14 -> z13 -> z12 so it works while tiles are still loading.
/// @see https://docs.mapbox.com/data/tilesets/reference/mapbox-terrain-rgb-v1/#elevation-data
float ground_height_at(entt::registry& registry, const Vector3& pos) {
  ResourceManager& rm = get_resource_manager(registry);
  for (const int zoom : {14, 13, 12}) {
    const float tile_size = tile_size_for_zoom(zoom);
    const int tx = static_cast<int>(std::floor(pos.x / tile_size));
    const int tz = static_cast<int>(std::floor(pos.z / tile_size));

    if (const int height_id = get_height_id(zoom, tx, tz); rm.images.contains(height_id)) {
      const Image& img = rm.images[height_id]->res;
      const float u = pos.x / tile_size - static_cast<float>(tx);
      const float v = pos.z / tile_size - static_cast<float>(tz);

      const int px = static_cast<int>(u * static_cast<float>(img.width));
      const int pz = static_cast<int>(v * static_cast<float>(img.height));

      return get_height_from_image(img, px, pz);
    }
  }
  return 0.0f;
}

/// terrain streamer responsible to render the "world" all the entities
/// leaves on. it supports multiple LOD (more level require performance
/// optimizations).
/// all resources are downloaded and cache automatically. currently support
/// mapbox api and require MAPBOX_TOKEN to be set as environment variable
class streamer {
  std::unordered_map<TileKey, entt::entity> desired_tiles;   // what LOD logic wants this frame
  std::unordered_map<TileKey, entt::entity> rendered_tiles;  // superset: desired + pending eviction
  Vector3 last_position{-9.9f, -9.9f, -9.9f};
  Shader displacement_shader;
  int cam_pos_loc = -1;
  int threads = 0;
  std::unique_ptr<Model> terrain_model12;
  std::unique_ptr<Model> terrain_model13;
  std::unique_ptr<Model> terrain_model14;
  ResourceManager& rmg;
  std::string token;

 public:
  explicit streamer(entt::registry& registry)
      : displacement_shader(LoadShader(resources::terrain_vertex_shader_path, resources::terrain_fragment_shader_path)),
        terrain_model12(std::make_unique<Model>(create_model(TILE_SIZE_12, 16))),
        terrain_model13(std::make_unique<Model>(create_model(TILE_SIZE_13, 64))),
        terrain_model14(std::make_unique<Model>(create_model(TILE_SIZE_14, 256))),
        rmg(get_resource_manager(registry)),
        token(get_options(registry).tiles_token) {
    // set the displacement_shader as the terrain model shader
    terrain_model12->materials[0].shader = displacement_shader;
    terrain_model13->materials[0].shader = displacement_shader;
    terrain_model14->materials[0].shader = displacement_shader;

    // keep for layer use
    cam_pos_loc = GetShaderLocation(displacement_shader, "cameraPosition");

    // set the heightmap data into MATERIAL_MAP_ROUGHNESS slot
    constexpr int heightmapSlotIndex = MATERIAL_MAP_ROUGHNESS;  // Raylib map roughness index
    const int shaderLocation = GetShaderLocation(displacement_shader, "heightMap");
    SetShaderValue(displacement_shader, shaderLocation, &heightmapSlotIndex, SHADER_UNIFORM_INT);

    // set scale 1 as long as the model is stretched properly
    constexpr float heightScale = 1.0;
    const int scaleLoc = GetShaderLocation(displacement_shader, "heightScale");
    SetShaderValue(displacement_shader, scaleLoc, &heightScale, SHADER_UNIFORM_FLOAT);
  }

  ~streamer() {
    UnloadModel(*terrain_model12);
    UnloadModel(*terrain_model13);
    UnloadModel(*terrain_model14);
    UnloadShader(displacement_shader);
  }

  void stream(entt::registry& registry, const Camera3D& camera) const {
    const auto& offset = get_player(registry).offset;
    SetShaderValue(displacement_shader, cam_pos_loc, &camera.position, SHADER_UNIFORM_VEC3);

    // we use BLEND_ALPHA to allow transparency in 3d (raylib default is not to allow)
    // we use transparency in shader to mimic fog
    // BeginBlendMode(BLEND_ALPHA);
    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      // the texture is not exists (not suppose to happen, just for safety)
      if (!rmg.textures.contains(chunk.model)) continue;
      // the heightmap is not exists (not suppose to happen, just for safety)
      if (!rmg.textures.contains(chunk.height)) continue;

      // now this access is safe
      const auto tex = rmg.textures[chunk.model]->res;
      const auto heightmap = rmg.textures[chunk.height]->res;

      // attach the texture and the heightmap to the slot we defined in the ctr
      const auto& model = model_for_zoom(chunk.zoom);
      model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = tex;
      model.materials[0].maps[MATERIAL_MAP_ROUGHNESS].texture = heightmap;

      DrawModel(model, pos.pos + offset, 1.0f, WHITE);
    }
    // EndBlendMode();
  }

  void show_debug_data() const {
    DrawText(TextFormat("desired tiles: %d", static_cast<int>(desired_tiles.size())), 15, 280, 10, BLACK);
    DrawText(TextFormat("rendered tiles: %d", static_cast<int>(rendered_tiles.size())), 15, 295, 10, BLACK);
    DrawText(TextFormat("threads: %d", threads), 15, 310, 10, BLACK);
  }

  void update(entt::registry& registry) {
    // remove from rendered not needed tiles
    std::erase_if(rendered_tiles, [&](const auto& item) {
      const auto [key, entity] = item;
      if (desired_tiles.contains(key)) return false;
      return is_tile_out_of_range(key) || is_tile_covered(key);
    });

    // remove orphans
    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      if (const auto key = TileKey{chunk.zoom, chunk.x, chunk.z}; !desired_tiles.contains(key) && !rendered_tiles.contains(key)) {
        unload_tile_resources(rmg, key.zoom, key.x, key.z);
        registry.destroy(entity);
      }
    }

    const auto player_pos = get_player(registry).abs_pos;

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
      if (const float dist_sq = tile_distance(player_pos, zoom, tx, tz);
          zoom == 14 || (zoom == 13 && dist_sq >= Z14_THRESHOLD_SQ) || (zoom == 12 && dist_sq >= Z13_THRESHOLD_SQ)) {
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
      return !std::ranges::binary_search(new_desired_keys, key);
    });

    // spawn new
    for (const auto& key : new_desired_keys) {
      if (!desired_tiles.contains(key)) desired_tiles[key] = spawn_tile(registry, key);
    }
  }

  void process_loaded_chunks(entt::registry& registry) {
    for (const auto view = registry.view<AsyncTileLoad>(); const auto [entity, tile] : view.each()) {
      const bool tex_ready = tile.texture_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
      const bool height_ready = tile.heightmap_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;

      if (!tex_ready || !height_ready) continue;

      // it is important to offload tile as soon as possible and
      // for sure before removing AsyncTileLoad from the entity
      const auto [texture_future, heightmap_future, x, z, zoom] = tile;
      const Image tex_img = texture_future.get();
      Image height_img = heightmap_future.get();
      threads -= 2;

      if (!IsImageValid(tex_img) || !IsImageValid(height_img)) {
        TraceLog(LOG_WARNING, "failed to load tile %d/%d/%d - the tile will not be display in next frame", zoom, x, z);
        // registry.remove<AsyncTileLoad>(entity);
        registry.destroy(entity);
        continue;
      }

      const Texture2D texture_tex = LoadTextureFromImage(tex_img);
      const Texture2D height_tex = LoadTextureFromImage(height_img);

      // need no more
      UnloadImage(tex_img);

      const auto texture_id = get_tex_id(zoom, x, z);
      const auto height_id = get_height_id(zoom, x, z);

      // verify we are attaching to free spaced
      unload_tile_resources(rmg, zoom, x, z);

      // the assign
      rmg.textures.load(texture_id, texture_tex);
      rmg.textures.load(height_id, height_tex);
      rmg.images.load(height_id, height_img);

      // component for rendering
      registry.emplace<TerrainChunk>(entity, texture_id, height_id, zoom, x, z);
      registry.emplace<TerrainHeight>(entity, height_id);
      registry.remove<AsyncTileLoad>(entity);

      // keep in map of rendered
      rendered_tiles[{zoom, x, z}] = entity;
      break;
    }
  }

  // debugging code
  // don't compile if not in use

  // static void draw_tile_labels(entt::registry& registry, const Camera3D& camera) {
  //   const auto& player = get_player(registry);
  //   const auto width = static_cast<float>(GetScreenWidth());
  //   const auto height = static_cast<float>(GetScreenHeight());
  //
  //   for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
  //     // Raise the label a bit above the ground for legibility
  //     const Vector3 world_pos = pos.pos + player.offset + Vector3{0.0f, 200.0f, 0.0f};
  //
  //     // "In front of camera" filter via dot product with forward
  //     if (const Vector3 to_tile = world_pos - camera.position; Vector3DotProduct(to_tile, player.forward) <= 0.0f) continue;
  //
  //     const Vector2 sp = GetWorldToScreen(world_pos, camera);
  //     if (sp.x < 0.0f || sp.x > width || sp.y < 0.0f || sp.y > height) continue;
  //
  //     DrawText(TextFormat("%d", chunk.zoom), static_cast<int>(sp.x), static_cast<int>(sp.y), 15, GREEN);
  //     DrawText(TextFormat("%d %d", chunk.x, chunk.z), static_cast<int>(sp.x) + 20, static_cast<int>(sp.y), 10, GREEN);
  //   }
  // }

  // void stream_debug(entt::registry& registry, const Camera3D& camera) const {
  //   const auto& player = get_player(registry);
  //   for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
  //     auto color = RED;
  //     auto size = TILE_SIZE_12;
  //     if (chunk.zoom == 13) {
  //       color = GREEN;
  //       size = TILE_SIZE_13;
  //     }
  //     if (chunk.zoom == 14) {
  //       color = BLUE;
  //       size = TILE_SIZE_14;
  //     }
  //     DrawCube(pos.pos + player.offset, size, 5.0f, size, color);
  //   }
  // }

 private:
  entt::entity spawn_tile(entt::registry& registry, const TileKey& tile) {
    const auto scale = 1 << (tile.zoom - ZOOM_LEVEL);
    const auto tx = tile.x + BASE_X * scale;
    const auto tz = tile.z + BASE_Z * scale;

    const auto tex_path = std::format("assets/tiles/texture/{}/{}/{}.png", tile.zoom, tx, tz);
    const auto height_path = std::format("assets/tiles/heightmaps/{}/{}/{}.png", tile.zoom, tx, tz);

    const auto tex_url = tile_downloader::texture_url(tile.zoom, tx, tz, token);
    const auto height_url = tile_downloader::heightmap_url(tile.zoom, tx, tz, token);

    const float tile_size = tile_size_for_zoom(tile.zoom);
    const float world_x = (static_cast<float>(tile.x) + 0.5f) * tile_size;
    const float world_z = (static_cast<float>(tile.z) + 0.5f) * tile_size;

    auto tex_future = tile_downloader::enqueue_and_load(tex_path, tex_url);
    auto height_future = tile_downloader::enqueue_and_load(height_path, height_url);
    threads += 2;

    const auto entity = registry.create();
    registry.emplace<Position3D>(entity, (Vector3){world_x, 0.0f, world_z});
    registry.emplace<AsyncTileLoad>(entity, std::move(tex_future), std::move(height_future), tile.x, tile.z, tile.zoom);

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

  [[nodiscard]] bool is_tile_out_of_range(const TileKey& key) const {
    const auto distance_sq = tile_distance(last_position, key.zoom, key.x, key.z);
    if (key.zoom == 14) return distance_sq > Z14_THRESHOLD_SQ;
    if (key.zoom == 13) return distance_sq > Z13_THRESHOLD_SQ;
    return distance_sq > RENDER_RADIUS_SQ;
  }

  [[nodiscard]] bool is_tile_covered(const TileKey& key) const {
    const auto contains = [&](const int zoom, const int x, const int z) { return rendered_tiles.contains(TileKey{zoom, x, z}); };

    if (key.zoom == 14) {
      const int parent_x = key.x >> 1;
      const int parent_z = key.z >> 1;
      return contains(13, parent_x, parent_z) || contains(12, parent_x >> 1, parent_z >> 1);
    }

    if (key.zoom == 13) {
      if (contains(12, key.x >> 1, key.z >> 1)) return true;

      const int child_x = key.x << 1;
      const int child_z = key.z << 1;
      return contains(14, child_x, child_z) && contains(14, child_x + 1, child_z) && contains(14, child_x, child_z + 1) &&
             contains(14, child_x + 1, child_z + 1);
    }

    if (key.zoom == 12) {
      const int child_x = key.x << 1;
      const int child_z = key.z << 1;
      if (contains(13, child_x, child_z) && contains(13, child_x + 1, child_z) && contains(13, child_x, child_z + 1) &&
          contains(13, child_x + 1, child_z + 1)) {
        return true;
      }

      const int grand_child_x = key.x << 2;
      const int grand_child_z = key.z << 2;
      for (int dx = 0; dx < 4; ++dx) {
        for (int dz = 0; dz < 4; ++dz) {
          if (!contains(14, grand_child_x + dx, grand_child_z + dz)) return false;
        }
      }
      return true;
    }

    return false;
  }
};
}  // namespace terrain_streamer
