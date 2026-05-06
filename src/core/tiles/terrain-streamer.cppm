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

struct streamer_config {
  // supported zoom
  int base_zoom = 11;
  int max_zoom = 14;

  // TMS anchor
  int base_x = 1223;
  int base_z = 828;

  Meter base_zoom_tile_size = 16600.0f;

  // the distance between tiles check
  MeterSq update_threshold = 2000.0f * 2000.0f;

  // the height change between tiels check
  Meter update_height_threshold = 500.0f;

  // rendering radius in base zoom tiles
  int rendering_radius = 7;

  // how much stretch the model to cover the stitches
  Meter skirt_size = 0.05f;

  // by zoom
  std::map<int, Meter> tile_sizes = {};
  std::map<int, Meter> tile_distances = {};

  explicit streamer_config() {
    const auto distance = rendering_radius * base_zoom_tile_size;

    for (int zoom = base_zoom; zoom <= max_zoom; ++zoom) {
      const int ratio = 1 << (zoom - base_zoom);
      tile_sizes[zoom] = base_zoom_tile_size / static_cast<float>(ratio);
      tile_distances[zoom] = distance * distance / static_cast<float>(ratio * ratio);
    }
  }

  [[nodiscard]] Meter get_size(const int zoom) const { return tile_sizes.at(zoom); }
  [[nodiscard]] Meter get_distance(const int zoom) const { return tile_distances.at(zoom); }
};

/// terrain streamer responsible to render the "world" all the entities
/// leaves on. it supports multiple LOD (more level require performance
/// optimizations).
/// all resources are downloaded and cache automatically. currently support
/// mapbox api and require MAPBOX_TOKEN to be set as environment variable
class streamer {
  streamer_config config{};
  std::unordered_map<TileKey, entt::entity> desired_tiles;   // what LOD logic wants this frame
  std::unordered_map<TileKey, entt::entity> rendered_tiles;  // superset: desired + pending eviction
  Vector3 last_position{-9.9f, -9.9f, -9.9f};

  int cam_pos_loc = -1;
  int ambient_loc = -1;
  int fog_color_log = -1;
  std::map<int, Model> models;

  Shader displacement_shader;
  ResourceManager& rmg;
  std::string token;

 public:
  explicit streamer(entt::registry& registry)
      : displacement_shader(LoadShader(resources::terrain_vertex_shader_path, resources::terrain_fragment_shader_path)),
        rmg(get_resource_manager(registry)),
        token(get_options(registry).get_tiles_token()) {
    // create default models for each zoom level
    // in the right size to avoid stretching models
    for (int zoom = config.base_zoom; zoom <= config.max_zoom; ++zoom) {
      const int ratio = 1 << (zoom - config.base_zoom);  // 1,2,4,8,...
      const int res = 16 * ratio;
      const float size = config.get_size(zoom);
      float skirt_size = size * config.skirt_size;  // / ratio;
      if (zoom == 11) skirt_size *= 3.0; // zoom 11 for some reason require larger skirt
      models[zoom] = LoadModelFromMesh(GenMeshPlane(size + skirt_size, size + skirt_size, res, res));
      models[zoom].materials[0].shader = displacement_shader;
    }

    // keep for later use in renderer (cache)
    cam_pos_loc = GetShaderLocation(displacement_shader, "cameraPosition");
    ambient_loc = GetShaderLocation(displacement_shader, "ambientLight");
    fog_color_log = GetShaderLocation(displacement_shader, "fogColor");

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
    for (const auto& val : models | std::views::values) UnloadModel(val);
    UnloadShader(displacement_shader);
  }

  void stream(entt::registry& registry, const Camera3D& camera) const {
    const auto& player = get_player(registry);
    auto& options = get_options(registry);
    // set the camera location (for distance -> fog)
    SetShaderValue(displacement_shader, cam_pos_loc, &camera.position, SHADER_UNIFORM_VEC3);

    // set the ambient color (weather/day/night/...)
    auto [x, y, z] = options.get_ambient_color();
    const float ambient_light[4] = {x, y, z, 1.0f};
    SetShaderValue(displacement_shader, ambient_loc, ambient_light, SHADER_UNIFORM_VEC4);

    // set the fog color (to match the sky)
    auto [hx, hy, hz] = options.get_horizon_color();
    const float fog_color[4] = {hx, hy, hz, 1.0f};
    SetShaderValue(displacement_shader, fog_color_log, &fog_color, SHADER_UNIFORM_VEC4);

    // we use BLEND_ALPHA to allow transparency in 3d (raylib default is not to allow)
    // we use transparency in shader to mimic fog
    // BeginBlendMode(BLEND_ALPHA);
    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      // do not render if it is not in front of the aircraft
      // with exception for close tile that always should be rendered
      const auto model_position = pos.pos + player.offset;
      const auto player_pos = player.pos;
      if (const auto to_tile = Vector3Normalize(model_position - player_pos);
          Vector3Distance(model_position, player_pos) > 2000.0f && Vector3DotProduct(to_tile, player.forward) <= -0.3f)
        continue;

      // the texture is not exists (not suppose to happen, just for safety)
      if (!rmg.textures.contains(chunk.model)) continue;
      // the heightmap is not exists (not suppose to happen, just for safety)
      if (!rmg.textures.contains(chunk.height)) continue;

      // now this access is safe the resources
      // attach the texture and the heightmap to the slot we defined in the ctr
      const auto& model = models.at(chunk.zoom);  // model_for_zoom(chunk.zoom);
      model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = rmg.textures[chunk.model]->res;
      model.materials[0].maps[MATERIAL_MAP_ROUGHNESS].texture = rmg.textures[chunk.height]->res;

      DrawModel(model, model_position, 1.0f, WHITE);
      // const auto size = TILE_SIZES.at(chunk.zoom);
      // DrawCubeWires(model_position, size, 500.0f, size, YELLOW);
    }
    // EndBlendMode();
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

    if (Vector3DistanceSqr(player_pos, last_position) < UPDATE_THRESHOLD && std::abs(player_pos.y - last_position.y) < UPDATE_HEIGHT_THRESHOLD)
      return;  // only update when moved more than N meters or gain/loose enough height
    last_position = player_pos;

    const int current_tile_x = static_cast<int>(std::floor(player_pos.x / config.base_zoom_tile_size));
    const int current_tile_z = static_cast<int>(std::floor(player_pos.z / config.base_zoom_tile_size));

    // build list or desired tiles keys
    std::vector<TileKey> new_desired_keys;
    new_desired_keys.reserve(512);

    // recursively subdivide a tile if it's within the threshold for the next zoom.
    // at max zoom (14) always add the leaf. otherwise, check the tile center distance.
    auto subdivide = [&](auto& self, const int zoom, const int tx, const int tz) -> void {
      if (const float dist_sq = tile_distance(player_pos, zoom, tx, tz); zoom == 14 || (zoom == 13 && dist_sq >= Z14_THRESHOLD_SQ) ||
                                                                         (zoom == 12 && dist_sq >= Z13_THRESHOLD_SQ) ||
                                                                         (zoom == 11 && dist_sq >= Z12_THRESHOLD_SQ)) {
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

    // height base optimization
    // todo move rlSetClipPlanes here and set it by height
    // todo not suppose to change the radius of the search, should change the threshold
    auto radius = 7;
    if (player_pos.y < 500)
      radius = 3;
    else if (player_pos.y < 1000)
      radius = 4;
    else if (player_pos.y < 2000)
      radius = 5;
    else if (player_pos.y < 4000)
      radius = 6;

    for (int dx = -radius; dx <= radius; ++dx) {
      for (int dz = -radius; dz <= radius; ++dz) {
        if (dz * dz + dx * dx > RENDER_DISC_R2) continue;
        subdivide(subdivide, 11, current_tile_x + dx, current_tile_z + dz);
      }
    }

    // sort for searching (still cheaper than set? YES, set required the heap and O(n log n) for searching)
    std::ranges::sort(new_desired_keys);

    TraceLog(LOG_INFO, "tile %d", new_desired_keys.size());

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
      // it is important to offload tile as soon as possible and
      // for sure before removing AsyncTileLoad from the entity
      const auto [texture_future, heightmap_future, x, z, zoom] = tile;

      // don't create stale tiles...
      // todo still need to consume the future to free the thread, but we can skip the rest of the process if not desired anymore
      // if (const TileKey key{zoom, x, z}; !desired_tiles.contains(key) && !rendered_tiles.contains(key)) {
      //   registry.destroy(entity);
      //   continue;
      // }

      if (const bool tex_ready = texture_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready; !tex_ready) continue;
      if (const bool height_ready = heightmap_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready; !height_ready) continue;

      const Image tex_img = texture_future.get();
      Image height_img = heightmap_future.get();

      if (!IsImageValid(tex_img) || !IsImageValid(height_img)) {
        TraceLog(LOG_WARNING, "failed to load tile %d/%d/%d - the tile will not be display in next frame", zoom, x, z);
        UnloadImage(tex_img);
        UnloadImage(height_img);
        registry.destroy(entity);
        continue;
      }

      // if the tile is already stale, no need to make so much work...
      if (const TileKey key{zoom, x, z}; !desired_tiles.contains(key) && !rendered_tiles.contains(key)) {
        TraceLog(LOG_INFO, "tile %d/%d/%d is already stale when loaded - the tile will not be display and resources will be freed", zoom, x, z);
        UnloadImage(tex_img);
        UnloadImage(height_img);
        registry.destroy(entity);
        continue;
      }

      // now it safe to do the heavy load..
      const Texture2D texture_tex = LoadTextureFromImage(tex_img);
      const Texture2D height_tex = LoadTextureFromImage(height_img);

      // need no more (keeping the heightmap for querying the ground height)
      UnloadImage(tex_img);

      const auto texture_id = get_tex_id(zoom, x, z);
      const auto height_id = get_height_id(zoom, x, z);

      // verify we are attaching to freed space (just for safety, should not happen)
      unload_tile_resources(rmg, zoom, x, z);

      // the assign
      rmg.textures.load(texture_id, texture_tex);
      rmg.textures.load(height_id, height_tex);
      rmg.images.load(height_id, height_img);

      // component for rendering
      registry.emplace<TerrainChunk>(entity, texture_id, height_id, zoom, x, z);
      registry.emplace<TerrainHeight>(entity, height_id);
      // remove the async tile
      registry.remove<AsyncTileLoad>(entity);

      // keep in map of rendered
      rendered_tiles[{zoom, x, z}] = entity;

      // we'll continue the next frame, the render is waiting
      break;
    }
  }

  // debugging code
  // don't compile if not in use

  // void show_debug_data() const {
  //   DrawText(TextFormat("desired tiles: %d", static_cast<int>(desired_tiles.size())), 15, 280, 10, BLACK);
  //   DrawText(TextFormat("rendered tiles: %d", static_cast<int>(rendered_tiles.size())), 15, 295, 10, BLACK);
  //   DrawText(TextFormat("threads: %d", threads), 15, 310, 10, BLACK);
  // }
  static void draw_tile_labels(entt::registry& registry, const Camera3D& camera) {
    const auto& player = get_player(registry);
    const auto width = static_cast<float>(GetScreenWidth());
    const auto height = static_cast<float>(GetScreenHeight());

    for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto [entity, chunk, pos] : view.each()) {
      // Raise the label a bit above the ground for legibility
      const Vector3 world_pos = pos.pos + player.offset + Vector3{0.0f, 200.0f, 0.0f};

      // "In front of camera" filter via dot product with forward
      if (const Vector3 to_tile = world_pos - camera.position; Vector3DotProduct(to_tile, player.forward) <= 0.0f) continue;

      const Vector2 sp = GetWorldToScreen(world_pos, camera);
      if (sp.x < 0.0f || sp.x > width || sp.y < 0.0f || sp.y > height) continue;

      DrawText(TextFormat("%d", chunk.zoom), static_cast<int>(sp.x), static_cast<int>(sp.y), 15, GREEN);
      DrawText(TextFormat("%d %d", chunk.x, chunk.z), static_cast<int>(sp.x) + 20, static_cast<int>(sp.y), 10, GREEN);
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

 private:
  entt::entity spawn_tile(entt::registry& registry, const TileKey& tile) const {
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

    const auto entity = registry.create();
    registry.emplace<Position3D>(entity, (Vector3){world_x, 0.0f, world_z});
    registry.emplace<AsyncTileLoad>(entity, std::move(tex_future), std::move(height_future), tile.x, tile.z, tile.zoom);

    return entity;
  }

  [[nodiscard]] bool is_tile_out_of_range(const TileKey& key) const {
    return tile_distance(last_position, key.zoom, key.x, key.z) > tile_threshold_for_zoom(key.zoom);
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

    if (key.zoom == 11) {
      const int child_x = key.x << 1;
      const int child_z = key.z << 1;
      if (contains(12, child_x, child_z) && contains(12, child_x + 1, child_z) && contains(12, child_x, child_z + 1) &&
          contains(12, child_x + 1, child_z + 1)) {
        return true;
      }

      const int grand_child_x = key.x << 2;
      const int grand_child_z = key.z << 2;
      for (int dx = 0; dx < 4; ++dx) {
        for (int dz = 0; dz < 4; ++dz) {
          if (!contains(13, grand_child_x + dx, grand_child_z + dz)) return false;
        }
      }
      return true;
    }

    return false;
  }
};
}  // namespace terrain_streamer
