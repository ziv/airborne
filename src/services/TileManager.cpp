#include "TileManager.h"


int TileManager::getTileID(int x, int z) {
    return x * 100 + z;
}

void TileManager::draw() {
    for (auto &pair: activeTiles) {
        TerrainChunk *chunk = pair.second.get();

        if (chunk->state == TileState::READY) {
            // -- חישוב מיקום הציור בעולם --
            // חשוב לדעת: הפונקציה GenMeshHeightmap של Raylib ממקמת את ציר האמצע של המודל
            // בדיוק במרכז שלו (ולא בפינה). לכן, אנחנו צריכים לחשב את המרכז של האריח.

            float centerX = (chunk->gridX * WorldConfig::CHUNK_WORLD_SIZE) + (WorldConfig::CHUNK_WORLD_SIZE / 2.0f);
            float centerZ = (chunk->gridZ * WorldConfig::CHUNK_WORLD_SIZE) + (WorldConfig::CHUNK_WORLD_SIZE / 2.0f);

            Vector3 position = {centerX, 0.0f, centerZ};

            // ציור המודל (הטקסטורה והגובה כבר מחוברים אליו מראש!)
            DrawModel(chunk->chunkModel, position, 1.0f, WHITE);

            // בונוס לאיפוס/דיבאג: אם אתה רוצה לראות את גבולות האריחים במפורש,
            // אפשר לצייר קופסת גבול אדומה סביבם:
            DrawBoundingBox(GetModelBoundingBox(chunk->chunkModel), RED);
        }
    }
}

void TileManager::update(Vector3 playerPos) {
    // 1. חישוב: באיזה אריח המטוס נמצא כרגע?
    // מחלקים את המיקום בעולם בגודל הפיזי של אריח (32.7 קילומטר במקרה שלנו!)
    int currentX = (int) floor(playerPos.x / WorldConfig::CHUNK_WORLD_SIZE);
    int currentZ = (int) floor(playerPos.z / WorldConfig::CHUNK_WORLD_SIZE);

    std::vector<int> requiredIDs;

    // 2. סריקת הרדיוס (רשת של 3x3 סביב המטוס)
    for (int x = currentX - WorldConfig::RENDER_RADIUS; x <= currentX + WorldConfig::RENDER_RADIUS; x++) {
        for (int z = currentZ - WorldConfig::RENDER_RADIUS; z <= currentZ + WorldConfig::RENDER_RADIUS; z++) {
            // הגנה קריטית: לא לצאת מגבולות המפה! (0 עד 15)
            if (x < 0 || x >= WorldConfig::GRID_SIZE || z < 0 || z >= WorldConfig::GRID_SIZE) continue;

            int tileID = getTileID(x, z);
            requiredIDs.push_back(tileID);

            // אם האריח לא קיים בזיכרון, ניצור אותו ונתחיל לטעון
            if (activeTiles.find(tileID) == activeTiles.end()) {
                auto chunk = std::make_unique<TerrainChunk>(x, z);

                // שיגור תהליך (Thread) 1: קריאת מפת הגובה (128x128)
                chunk->heightmapFuture = std::async(std::launch::async, [x, z]() {
                    std::string path = TextFormat("res/il/hm-parts/north-hm-%d-%d.png", x, z);
                    return LoadImage(path.c_str());
                });

                // שיגור תהליך (Thread) 2: קריאת הטקסטורה של הלוויין (512x512)
                chunk->textureFuture = std::async(std::launch::async, [x, z]() {
                    std::string path = TextFormat("res/il/tx-parts/north-tx-%d-%d.png", x, z);
                    return LoadImage(path.c_str());
                });

                // הוספה למילון האריחים הפעילים. המצב כרגע הוא LOADING_IMAGES
                activeTiles[tileID] = std::move(chunk);
            }
        }
    }

    // 3. פינוי זבל: אם אריח נמצא ב-activeTiles אבל לא ב-requiredIDs, השחקן התרחק ממנו.
    for (auto it = activeTiles.begin(); it != activeTiles.end();) {
        bool isRequired = false;
        for (int id: requiredIDs) {
            if (it->first == id) {
                isRequired = true;
                break;
            }
        }

        if (!isRequired) {
            // המחיקה מפעילה אוטומטית את ה-Destructor שיצרנו בשלב 1 ומשחררת VRAM!
            it = activeTiles.erase(it);
        } else {
            ++it;
        }
    }

    // --- כאן יבוא השלב הבא: בדיקה אם התמונות סיימו להיטען והעלאתן ל-GPU ---
    checkAndBuildMeshes();
}

void TileManager::checkAndBuildMeshes() {
    for (auto &pair: activeTiles) {
        TerrainChunk *chunk = pair.second.get();

        // אנחנו מטפלים רק באריחים שכרגע נטענים
        if (chunk->state == TileState::LOADING_IMAGES) {
            // בודקים אם *גם* מפת הגובה ו*גם* הטקסטורה סיימו להיטען (בלי לחסום את המשחק)
            bool hmReady = chunk->heightmapFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
            bool txReady = chunk->textureFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready;

            if (hmReady && txReady) {
                TraceLog(LOG_ERROR, "%d %d", chunk->gridX, chunk->gridZ);
                chunk->state = TileState::BUILDING_MESH;

                // 1. שולפים את התמונות מה-Threads אל התהליך הראשי
                Image hmImage = chunk->heightmapFuture.get();
                Image txImage = chunk->textureFuture.get();

                // 2. מגדירים את הגודל הפיזי של המודל בתוך עולם המשחק
                // רוחב ועומק: 32,768 מטר. גובה מקסימלי: 2000 מטר.
                Vector3 modelSize = {
                    WorldConfig::CHUNK_WORLD_SIZE,
                    WorldConfig::MAX_TERRAIN_HEIGHT,
                    WorldConfig::CHUNK_WORLD_SIZE
                };

                // 3. מייצרים את הרשת (Mesh) ממפת הגובה (זה לוקח שבריר שנייה)
                Mesh terrainMesh = GenMeshHeightmap(hmImage, modelSize);

                // 4. מייצרים את המודל הרשמי ומלבישים עליו את הטקסטורה של הלוויין
                chunk->chunkModel = LoadModelFromMesh(terrainMesh);
                // TraceLog(LOG_ERROR, "Valid?");
                chunk->texture = LoadTextureFromImage(txImage);
                // TraceLog(LOG_ERROR, "Valid");

                // חיבור הטקסטורה לחומר (Material) של המודל כדי שכרטיס המסך ידע לצייר אותה
                chunk->chunkModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = chunk->texture;


                // 5. ניקיון קריטי! מוחקים את תמונות הגלם מזיכרון ה-RAM (הן כבר על ה-VRAM)
                UnloadImage(hmImage);
                UnloadImage(txImage);

                // האריח מוכן לציור!
                chunk->state = TileState::READY;
            }
        }
    }
}
