module;
#include <array>
#include <entt/entt.hpp>
#include <future>

#include "../../lib/ray.hpp"

export module TerrainStreaming:Utils;

import Types;
import ResourceManager;

constexpr Meter UPDATE_THRESHOLD = 200.0f * 200.0f;
constexpr Meter SKIRT_SIZE = 0.0f;
constexpr Meter TILE_SIZE_12 = 9783.9f;
constexpr Meter TILE_SIZE_13 = 4891.95f;
constexpr Meter TILE_SIZE_14 = 2445.975f;
constexpr Meter TILE_SIZE_15 = 1222.9875f;
// constexpr std::array<Meter, 3> TILE_SIZES{TILE_SIZE_12, TILE_SIZE_13, TILE_SIZE_14};

constexpr int ZOOM_LEVEL = 12;
constexpr int BASE_X = 2444;
constexpr int BASE_Z = 1655;

constexpr int RENDER_DISC_R2 = 36;
constexpr Meter RENDER_RADIUS = 6 * TILE_SIZE_12;
constexpr Meter RENDER_RADIUS_SQ = RENDER_RADIUS * RENDER_RADIUS;
constexpr Meter Z13_THRESHOLD = RENDER_RADIUS * 0.5f;
constexpr Meter Z14_THRESHOLD = RENDER_RADIUS * 0.25f;
constexpr Meter Z13_THRESHOLD_SQ = Z13_THRESHOLD * Z13_THRESHOLD;
constexpr Meter Z14_THRESHOLD_SQ = Z14_THRESHOLD * Z14_THRESHOLD;
constexpr Meter Z15_THRESHOLD_SQ = 2500.0f * 2500.0f;

struct TileKey {
  int zoom;
  int x;
  int z;
  auto operator<=>(const TileKey&) const = default;
};

// module private methods

constexpr Meter tile_size_for_zoom(const int zoom) { return TILE_SIZE_12 / static_cast<Meter>(1 << (zoom - ZOOM_LEVEL)); }

// ids for textures and heightmap to share between components
int get_tex_id(const int zoom, const int x, const int z) { return entt::hashed_string(TextFormat("tile_tex_%d_%d_%d", zoom, x, z)); }
int get_height_id(const int zoom, const int x, const int z) { return entt::hashed_string(TextFormat("tile_height_%d_%d_%d", zoom, x, z)); }

// remove tile's dependencies
void unload_tile_resources(ResourceManager& rm, const int zoom, const int x, const int z) {
  rm.textures.erase(get_tex_id(zoom, x, z));
  rm.textures.erase(get_height_id(zoom, x, z));
  rm.images.erase(get_height_id(zoom, x, z));
}

// create models for tiles (model per zoom)
Model create_model(const Meter size) { return LoadModelFromMesh(GenMeshPlane(size + size * 0.02f, size + size * 0.02f, 256, 256)); }

float tile_distance(const Vector3& player_pos, const int zoom, const int tx, const int tz) {
  const float tile_size = tile_size_for_zoom(zoom);
  const float world_x = (static_cast<float>(tx) + 0.5f) * tile_size;
  const float world_z = (static_cast<float>(tz) + 0.5f) * tile_size;
  const float ddx = player_pos.x - world_x;
  const float ddz = player_pos.z - world_z;
  return ddx * ddx + ddz * ddz;
}

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
