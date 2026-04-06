#pragma once
#include "raylib.h"
#include <string>
#include <map>
#include <vector>
#include <future>
#include <chrono>
#include <memory>

namespace WorldConfig {
    const int GRID_SIZE = 16; // סך הכל 16x16 אריחים (0 עד 15)

    const int HM_CHUNK_SIZE = 128; // פיקסלים לאריח מפת גובה
    const int TX_CHUNK_SIZE = 512; // פיקסלים לאריח טקסטורה

    // const float PIXELS_TO_METERS = 256.0f; // קנה המידה הפיזי
    const float PIXELS_TO_METERS = 16.0f; // קנה המידה הפיזי

    // הגודל הפיזי של כל אריח במנוע המשחק (128 * 256 = 32768.0)
    const float CHUNK_WORLD_SIZE = HM_CHUNK_SIZE * PIXELS_TO_METERS;

    const int RENDER_RADIUS = 2; // כמה אריחים לטעון מסביב למטוס (1 = רדיוס של 3x3)

    const float MAX_TERRAIN_HEIGHT = 2000.0f;
}

enum class TileState {
    LOADING_IMAGES, // השלב האסינכרוני: קורא קבצי PNG מהכונן אל זיכרון ה-RAM
    BUILDING_MESH, // השלב הסינכרוני: מעלה ל-GPU, יוצר Mesh וטקסטורה
    READY // האריח מוכן לציור על המסך
};

struct TerrainChunk {
    int gridX;
    int gridZ;
    TileState state;

    // -- נתונים זמניים לשלב ה-RAM (נטענים ברקע) --
    std::future<Image> heightmapFuture;
    std::future<Image> textureFuture;

    // -- נתונים סופיים לשלב ה-GPU (לציור) --
    Model chunkModel; // המודל התלת-ממדי שיווצר ממפת הגובה
    Texture2D texture; // הטקסטורה שתוצמד למודל

    // בנאי מאתחל
    TerrainChunk(int x, int z) : gridX(x), gridZ(z), state(TileState::LOADING_IMAGES) {
    }

    // Destructor (פונקציית הורס) - קריטית למניעת דליפות זיכרון ב-VRAM!
    ~TerrainChunk() {
        if (state == TileState::READY || state == TileState::BUILDING_MESH) {
            // שחרור המשאבים של כרטיס המסך כשהאריח נמחק
            UnloadTexture(texture);
            UnloadModel(chunkModel);
        }
    }
};

class TileManager {
public:
    std::map<int, std::unique_ptr<TerrainChunk> > activeTiles;

    static int getTileID(int x, int z);

    void draw();
    void checkAndBuildMeshes();
    void update(Vector3 playerPos);
};
