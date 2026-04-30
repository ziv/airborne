module;
#include <algorithm>
#include <entt/entt.hpp>
#include <format>
#include <future>
#include <map>
#include <string>
#include <vector>

#include "../../lib/ray.hpp"

export module TerrainStreaming:Utils;

import Types;

constexpr int ZOOM_LEVEL = 12;
constexpr Meter TILE_SIZE_12 = 9783.9f;

constexpr Meter tile_size_for_zoom(const int zoom) { return TILE_SIZE_12 / static_cast<Meter>(1 << (zoom - ZOOM_LEVEL)); }

export namespace terrain_streamer_utils {

Model create_model(const Meter size) { return LoadModelFromMesh(GenMeshPlane(size + size * 0.02f, size + size * 0.02f, 256, 256)); }

// world-space center for a tile at any zoom, aligned within its z12 parent area.
inline float tile_world_pos(const int zoom, const int local_idx) {
  const int n = 1 << (zoom - ZOOM_LEVEL);
  const int parent = local_idx >> (zoom - ZOOM_LEVEL);
  const int child = local_idx - parent * n;  // 0 .. n-1
  const Meter sz = tile_size_for_zoom(zoom);
  return static_cast<float>(parent) * TILE_SIZE_12 + (static_cast<float>(child) - (static_cast<float>(n) - 1.0f) * 0.5f) * sz;
}


// [[nodiscard]] bool is_tile_out_of_range(const TileKey& key) const {
//   const float tile_size = TILE_SIZE_12 / static_cast<float>(1 << (key.zoom - ZOOM_LEVEL));
//   const float world_x = (static_cast<float>(key.x) + 0.5f) * tile_size;
//   const float world_z = (static_cast<float>(key.z) + 0.5f) * tile_size;
//   const float ddx = last_position.x - world_x;
//   const float ddz = last_position.z - world_z;
//   const float dist_sq = ddx * ddx + ddz * ddz;
//   return dist_sq > RENDER_RADIUS * RENDER_RADIUS;
// }
}  // namespace terrain_streamer_utils