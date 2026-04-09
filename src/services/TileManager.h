/**
 * @file TileManager.h
 * @brief Streaming terrain tile system — loads heightmap/texture chunks asynchronously
 *        around the player and builds GPU meshes when data is ready.
 *
 * The world is divided into a 16×16 grid of tiles. Tiles within a configurable
 * render radius are loaded via std::async; once both images are ready they are
 * promoted to GPU models on the main thread. Tiles outside the radius are
 * evicted, freeing VRAM.
 */
#pragma once
#include "raylib.h"
#include <string>
#include <map>
#include <vector>
#include <future>
#include <chrono>
#include <memory>

/// @brief Global constants for the tile-based terrain system.
namespace WorldConfig {
    const int GRID_SIZE = 16;            ///< Total tile grid dimension (16×16).
    const int HM_CHUNK_SIZE = 128;       ///< Heightmap image resolution per tile (pixels).
    const int TX_CHUNK_SIZE = 512;       ///< Texture image resolution per tile (pixels).
    const float PIXELS_TO_METERS = 16.0f;///< Scale factor from heightmap pixels to world meters.

    /// Physical world size of one tile (HM_CHUNK_SIZE × PIXELS_TO_METERS).
    const float CHUNK_WORLD_SIZE = HM_CHUNK_SIZE * PIXELS_TO_METERS;

    const int RENDER_RADIUS = 2;             ///< Tile radius to load around the player.
    const float MAX_TERRAIN_HEIGHT = 2000.0f; ///< Maximum elevation in meters.
}

/// @brief Lifecycle state of a single terrain tile.
enum class TileState {
    LOADING_IMAGES, ///< Async threads are reading PNG files from disk into RAM.
    BUILDING_MESH,  ///< Images loaded; building GPU mesh and texture on the main thread.
    READY           ///< Tile is fully built and ready to draw.
};

/// @brief A single terrain tile with its async load futures and GPU resources.
struct TerrainChunk {
    int gridX;      ///< Tile X index in the world grid.
    int gridZ;      ///< Tile Z index in the world grid.
    TileState state;

    std::future<Image> heightmapFuture; ///< Async heightmap image load.
    std::future<Image> textureFuture;   ///< Async texture image load.

    Model chunkModel;    ///< GPU model generated from the heightmap.
    Texture2D texture;   ///< GPU texture applied to the model.

    TerrainChunk(int x, int z) : gridX(x), gridZ(z), state(TileState::LOADING_IMAGES) {
    }

    /// @brief Destructor — releases GPU resources (VRAM) when the tile is evicted.
    ~TerrainChunk() {
        if (state == TileState::READY || state == TileState::BUILDING_MESH) {
            UnloadTexture(texture);
            UnloadModel(chunkModel);
        }
    }
};

/**
 * @brief Manages streaming terrain tiles around the player position.
 *
 * Call update() each frame with the player's world position; it will start
 * async loads for nearby tiles, evict distant tiles, and promote loaded
 * tiles to GPU-ready state via checkAndBuildMeshes().
 */
class TileManager {
public:
    std::map<int, std::unique_ptr<TerrainChunk> > activeTiles; ///< Currently loaded tiles keyed by tile ID.

    /// @brief Compute a unique integer ID from grid coordinates.
    static int getTileID(int x, int z);

    /// @brief Draw all READY tiles.
    void draw();

    /// @brief Promote tiles from LOADING_IMAGES → READY once their async futures complete.
    void checkAndBuildMeshes();

    /// @brief Manage tile loading/unloading based on the player's world position.
    void update(Vector3 playerPos);
};
