/**
 * @file TileManager.cpp
 * @brief Async terrain tile streaming — load, build, draw, and evict.
 */
#include "TileManager.h"


int TileManager::getTileID(int x, int z) {
    return x * 100 + z;
}

void TileManager::draw() {
    for (auto &pair: activeTiles) {
        TerrainChunk *chunk = pair.second.get();

        if (chunk->state == TileState::READY) {
            // GenMeshHeightmap centres the model at its midpoint, so compute
            // the world-space centre of this tile from its grid indices.
            float centerX = (chunk->gridX * WorldConfig::CHUNK_WORLD_SIZE) + (WorldConfig::CHUNK_WORLD_SIZE / 2.0f);
            float centerZ = (chunk->gridZ * WorldConfig::CHUNK_WORLD_SIZE) + (WorldConfig::CHUNK_WORLD_SIZE / 2.0f);

            Vector3 position = {centerX, 0.0f, centerZ};

            DrawModel(chunk->chunkModel, position, 1.0f, WHITE);
            DrawBoundingBox(GetModelBoundingBox(chunk->chunkModel), RED);
        }
    }
}

void TileManager::update(Vector3 playerPos) {
    // 1. Determine which tile the player is currently standing on.
    int currentX = (int) floor(playerPos.x / WorldConfig::CHUNK_WORLD_SIZE);
    int currentZ = (int) floor(playerPos.z / WorldConfig::CHUNK_WORLD_SIZE);

    std::vector<int> requiredIDs;

    // 2. Scan the render radius and start async loads for missing tiles.
    for (int x = currentX - WorldConfig::RENDER_RADIUS; x <= currentX + WorldConfig::RENDER_RADIUS; x++) {
        for (int z = currentZ - WorldConfig::RENDER_RADIUS; z <= currentZ + WorldConfig::RENDER_RADIUS; z++) {
            // clamp to valid grid bounds (0 .. GRID_SIZE-1)
            if (x < 0 || x >= WorldConfig::GRID_SIZE || z < 0 || z >= WorldConfig::GRID_SIZE) continue;

            int tileID = getTileID(x, z);
            requiredIDs.push_back(tileID);

            // spawn a new tile if not already loaded
            if (activeTiles.find(tileID) == activeTiles.end()) {
                auto chunk = std::make_unique<TerrainChunk>(x, z);

                // async thread 1: load heightmap image from disk
                chunk->heightmapFuture = std::async(std::launch::async, [x, z]() {
                    std::string path = TextFormat("res/il/hm-parts/north-hm-%d-%d.png", x, z);
                    return LoadImage(path.c_str());
                });

                // async thread 2: load satellite texture from disk
                chunk->textureFuture = std::async(std::launch::async, [x, z]() {
                    std::string path = TextFormat("res/il/tx-parts/north-tx-%d-%d.png", x, z);
                    return LoadImage(path.c_str());
                });

                activeTiles[tileID] = std::move(chunk);
            }
        }
    }

    // 3. Evict tiles outside the render radius (destructor frees VRAM).
    for (auto it = activeTiles.begin(); it != activeTiles.end();) {
        bool isRequired = false;
        for (int id: requiredIDs) {
            if (it->first == id) {
                isRequired = true;
                break;
            }
        }

        if (!isRequired) {
            it = activeTiles.erase(it);
        } else {
            ++it;
        }
    }

    // 4. Promote tiles whose async images have finished loading.
    checkAndBuildMeshes();
}

/// @brief Check each LOADING_IMAGES tile; if both futures are ready, build
///        the GPU mesh and texture on the main thread.
void TileManager::checkAndBuildMeshes() {
    for (auto &pair: activeTiles) {
        TerrainChunk *chunk = pair.second.get();

        if (chunk->state == TileState::LOADING_IMAGES) {
            // non-blocking check: are both async image loads finished?
            bool hmReady = chunk->heightmapFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
            bool txReady = chunk->textureFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready;

            if (hmReady && txReady) {
                TraceLog(LOG_ERROR, "%d %d", chunk->gridX, chunk->gridZ);
                chunk->state = TileState::BUILDING_MESH;

                // retrieve images from the async threads
                Image hmImage = chunk->heightmapFuture.get();
                Image txImage = chunk->textureFuture.get();

                // define the physical size of this tile in the game world
                Vector3 modelSize = {
                    WorldConfig::CHUNK_WORLD_SIZE,
                    WorldConfig::MAX_TERRAIN_HEIGHT,
                    WorldConfig::CHUNK_WORLD_SIZE
                };

                // generate the terrain mesh from the heightmap
                Mesh terrainMesh = GenMeshHeightmap(hmImage, modelSize);

                // build the model and assign the satellite texture
                chunk->chunkModel = LoadModelFromMesh(terrainMesh);
                chunk->texture = LoadTextureFromImage(txImage);
                chunk->chunkModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = chunk->texture;

                // free the CPU-side images (data is now on the GPU)
                UnloadImage(hmImage);
                UnloadImage(txImage);

                chunk->state = TileState::READY;
            }
        }
    }
}
