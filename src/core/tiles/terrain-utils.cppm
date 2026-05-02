module;
#include <entt/entt.hpp>

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
constexpr Meter Z15_THRESHOLD = RENDER_RADIUS * 0.125;
constexpr Meter Z13_THRESHOLD_SQ = Z13_THRESHOLD * Z13_THRESHOLD;
constexpr Meter Z14_THRESHOLD_SQ = Z14_THRESHOLD * Z14_THRESHOLD;
constexpr Meter Z15_THRESHOLD_SQ = Z15_THRESHOLD * Z15_THRESHOLD;

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
Model create_model(const Meter size, const int res) { return LoadModelFromMesh(GenMeshPlane(size + size * 0.02f, size + size * 0.02f, res, res)); }

float tile_distance(const Vector3& player_pos, const int zoom, const int tx, const int tz) {
  const float tile_size = tile_size_for_zoom(zoom);
  const float world_x = (static_cast<float>(tx) + 0.5f) * tile_size;
  const float world_z = (static_cast<float>(tz) + 0.5f) * tile_size;
  const float ddx = player_pos.x - world_x;
  const float ddz = player_pos.z - world_z;
  return ddx * ddx + ddz * ddz;
}

/// a better performance function instead of using
/// const auto c = GetImageColor(img, px, pz);
inline float get_height_from_image(const Image& img, int x, int y) {
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x >= img.width) x = img.width - 1;
  if (y >= img.height) y = img.height - 1;

  const auto pixels = static_cast<const unsigned char*>(img.data);
  unsigned char r, g, b;

  if (img.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8) {
    const int index = (y * img.width + x) * 3;
    r = pixels[index];
    g = pixels[index + 1];
    b = pixels[index + 2];
  } else if (img.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
    const int index = (y * img.width + x) * 4;
    r = pixels[index];
    g = pixels[index + 1];
    b = pixels[index + 2];
  } else {
    return 0.0f;
  }

  return -10000.0f + (static_cast<float>(r) * 65536.0f + static_cast<float>(g) * 256.0f + static_cast<float>(b)) * 0.1f;
}