#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <vector>
#include <map>
#include <utility>
#include <cmath>
#include <iostream>
#include <memory>

#include "GameScreen.h"
#include "MainMenuScreen.h"
#include "SplashScreen.h"


constexpr float CAMERA_TILT_DOWN = 0.45f;

int main() {
    InitWindow(1280, 720, "SE");
    InitAudioDevice();
    SetTargetFPS(60);

    std::unique_ptr<GameScreen> currentScreen = std::make_unique<SplashScreen>();
    auto currentState = ScreenState::SPLASH;

    while (!WindowShouldClose()) {
        // screens state machine
        if (const ScreenState nextState = currentScreen->Update(); nextState != currentState) {
            switch (nextState) {
                case ScreenState::MAIN_MENU:
                    currentScreen = std::make_unique<MainMenuScreen>();
                    break;
                default:
                case ScreenState::SPLASH:
                    currentScreen = std::make_unique<SplashScreen>();
                    break;
            }
            currentState = nextState;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        currentScreen->Draw();
        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;

    // Camera3D camera = {0};
    // camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    // camera.fovy = 85.0f;
    // camera.projection = CAMERA_PERSPECTIVE;
    //
    // camera.position = (Vector3){0.0f, 100.0f, 0.0f};
    // camera.target = (Vector3){10.0f, 100.0f, 10.0f};
    //
    // Vector3 forward = {0, 0, 1};
    // Vector3 up = {0, 1, 0};
    // float currentSpeed = 0.f;
    //
    // SetTargetFPS(60);
    // rlSetClipPlanes(1.0f, 100.0f);
    //
    //
    // Image textureImage = LoadImage("texture.jpg");
    // const Texture2D texture = LoadTextureFromImage(textureImage);
    // UnloadImage(textureImage);
    //
    // Image heightImage = LoadImage("heightmap.png");
    // // Vector3 size = {10000.0f, 10000.0f, 10000.0f};
    // Vector3 size = {100.0f, 100.0f, 100.0f};
    // Mesh mesh = GenMeshHeightmap(heightImage, size);
    // UnloadImage(heightImage);
    //
    // Model model = LoadModelFromMesh(mesh);
    //
    // model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    //
    // Image cockpit = LoadImage("cockpit.png");
    // Texture2D cockpitTexture = LoadTextureFromImage(cockpit);
    //
    // while (!WindowShouldClose()) {
    //     float deltaTime = GetFrameTime();
    //
    //     // if (crashed) {
    //     //     BeginDrawing();
    //     //     ClearBackground(MAROON);
    //     //     DrawText("CRASHED!", 1280 / 2 - 100, 720 / 2, 50, WHITE);
    //     //     EndDrawing();
    //     //     continue;
    //     // }
    //
    //     // flight
    //     float pitch = 0.0f;
    //     float roll = 0.0f;
    //     float yaw = 0.0f;
    //     if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) pitch = -1.0f * deltaTime;
    //     if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) pitch = 1.0f * deltaTime;
    //     if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) roll = -2.0f * deltaTime;
    //     if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) roll = 2.0f * deltaTime;
    //     if (IsKeyDown(KEY_Q)) yaw = 0.5f * deltaTime;
    //     if (IsKeyDown(KEY_E)) yaw = -0.5f * deltaTime;
    //
    //     if (IsKeyDown(KEY_EQUAL) || IsKeyDown(KEY_TWO)) currentSpeed += 10.0f * deltaTime;
    //     if (IsKeyDown(KEY_MINUS) || IsKeyDown(KEY_ONE)) currentSpeed -= 10.0f * deltaTime;
    //     // if (currentSpeed < 50.0f) currentSpeed = 50.0f;
    //     if (currentSpeed > 4000.0f) currentSpeed = 4000.0f;
    //
    //     // reset nose
    //     if (IsKeyDown(KEY_R)) up = {0, 1, 0};
    //
    //     // find right normal
    //     Vector3 right = Vector3CrossProduct(forward, up);
    //     // calculate the forward using the right normal and pitch
    //     forward = Vector3RotateByAxisAngle(forward, right, pitch);
    //     // calculate the up pitch
    //     up = Vector3RotateByAxisAngle(up, right, pitch);
    //     // calculate the app roll
    //     up = Vector3RotateByAxisAngle(up, forward, roll);
    //
    //     // update forward and up yaw (not part of the flight)
    //     forward = Vector3RotateByAxisAngle(forward, (Vector3){0, 1, 0}, yaw);
    //     up = Vector3RotateByAxisAngle(up, (Vector3){0, 1, 0}, yaw);
    //
    //     // camera.position = Vector3Add(camera.position, Vector3Scale(forward, currentSpeed * deltaTime));
    //     // camera.target = Vector3Add(camera.position, forward);
    //     // camera.up = up;
    //
    //     // 1. הזזת המטוס (הפיזיקה נשארת נאמנה לוקטור ה-forward האמיתי של הטיסה)
    //     camera.position = Vector3Add(camera.position, Vector3Scale(forward, currentSpeed * deltaTime));
    //
    //     // 2. חישוב זווית הראייה של הטייס (מוטה למטה ביחס לאף המטוס)
    //     // אנחנו מסובבים את וקטור ה"קדימה" ווקטור ה"למעלה" סביב ציר ה"ימינה" (הכנפיים)
    //     Vector3 pilotLookDir = Vector3RotateByAxisAngle(forward, right, -CAMERA_TILT_DOWN);
    //     Vector3 pilotUpDir = Vector3RotateByAxisAngle(up, right, -CAMERA_TILT_DOWN);
    //
    //     // 3. עדכון המצלמה לפי וקטור המבט החדש
    //     camera.target = Vector3Add(camera.position, pilotLookDir);
    //     camera.up = pilotUpDir;
    //
    //
    //     // --- הסטת חלון הטעינה (Directional Streaming) ---
    //     // int currentChunkX = static_cast<int>(floor(camera.position.x / CHUNK_SIZE_XZ));
    //     // int currentChunkZ = static_cast<int>(floor(camera.position.z / CHUNK_SIZE_XZ));
    //
    //
    //     // סימון הכל כישן
    //     // for (auto &pair: activeChunks) pair.second.active = false;
    //
    //     // for (int z = currentChunkZ - VIEW_RADIUS; z <= currentChunkZ + VIEW_RADIUS; z++) {
    //     //     for (int x = currentChunkX - VIEW_RADIUS; x <= currentChunkX + VIEW_RADIUS; x++) {
    //     //         if (x >= 0 && x < GRID_MAX && z >= 0 && z < GRID_MAX) {
    //     //             auto key = std::make_pair(x, z);
    //     //             if (activeChunks.find(key) == activeChunks.end()) {
    //     //                 activeChunks[key] = LoadTerrainChunk(x, z, fogShader);
    //     //             }
    //     //             activeChunks[key].active = true;
    //     //         }
    //     //     }
    //     // }
    //
    //     // מחיקת ישנים (כאן אנחנו חייבים להשתמש באיטרטור, לכן נשאר pair.second)
    //     // for (auto it = activeChunks.begin(); it != activeChunks.end();) {
    //     //     if (!it->second.active) {
    //     //         UnloadModel(it->second.model);
    //     //         UnloadTexture(it->second.texture);
    //     //         it = activeChunks.erase(it);
    //     //     } else ++it;
    //     // }
    //
    //     // התרסקות
    //     // float groundHeight = -100.0f;
    //     // auto currentKey = std::make_pair(currentChunkX, currentChunkZ);
    //     // if (activeChunks.find(currentKey) != activeChunks.end()) {
    //     //     float localX = fmod(camera.position.x, CHUNK_SIZE_XZ);
    //     //     float localZ = fmod(camera.position.z, CHUNK_SIZE_XZ);
    //     //     int pixelX = static_cast<int>((localX / CHUNK_SIZE_XZ) * 255.0f);
    //     //     int pixelZ = static_cast<int>((localZ / CHUNK_SIZE_XZ) * 255.0f);
    //     //     if (pixelX >= 0 && pixelX < 256 && pixelZ >= 0 && pixelZ < 256) {
    //     //         groundHeight = activeChunks[currentKey].heightData[pixelX][pixelZ] * CHUNK_HEIGHT;
    //     //     }
    //     // }
    //     // if (camera.position.y <= groundHeight + 5.0f) crashed = true;
    //
    //     // --- ציור ---
    //     BeginDrawing();
    //     ClearBackground((Color){135, 206, 235, 255}); // תכלת שמתאים לערפל שלנו
    //
    //     BeginMode3D(camera);
    //     // ** שיפור C++ מודרני: קריא וברור, ללא pair.second! **
    //     // for (const auto &[coords, chunk]: activeChunks) {
    //     //     DrawModel(chunk.model, chunk.drawPosition, 1.0f, WHITE);
    //     // }
    //     DrawModel(model, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
    //     DrawGrid(100, 10.0f);
    //     EndMode3D();
    //
    //     DrawText(TextFormat("Cx: %0.0f", camera.position.x), 20, 20, 20, LIME);
    //     DrawText(TextFormat("Cy: %0.0f", camera.position.y), 20, 40, 20, LIME);
    //     DrawText(TextFormat("Cz: %0.0f", camera.position.z), 20, 60, 20, LIME);
    //
    //     DrawText(TextFormat("Tx: %0.0f", camera.target.x), 20, 80, 20, YELLOW);
    //     DrawText(TextFormat("Ty: %0.0f", camera.target.y), 20, 100, 20, YELLOW);
    //     DrawText(TextFormat("Tz: %0.0f", camera.target.z), 20, 120, 20, YELLOW);
    //
    //     DrawTexture(cockpitTexture, 0, 100, WHITE);
    //
    //     // DrawText(TextFormat("ALT G: %0.0f", groundHeight), 20, 40, 20, LIME);
    //     // DrawText(TextFormat("SPD: %0.0f", currentSpeed), 20, 60, 20, LIME);
    //     // DrawText(TextFormat("SECTOR: X:%d Z:%d", currentChunkX, currentChunkZ), 20, 80, 20, YELLOW);
    //     // DrawText(TextFormat("ACTIVE CHUNKS: %zu", activeChunks.size()), 20, 100, 20, ORANGE);
    //
    //     DrawLine(1280 / 2 - 15, 720 / 2, 1280 / 2 + 15, 720 / 2, BLACK);
    //     DrawLine(1280 / 2, 720 / 2 - 15, 1280 / 2, 720 / 2 + 15, BLACK);
    //
    //     EndDrawing();
    // }
    //
    // // ניקוי ביציאה
    // // for (auto &pair: activeChunks) {
    // //     UnloadModel(pair.second.model);
    // //     UnloadTexture(pair.second.texture);
    // // }
    // // UnloadShader(fogShader);
    // UnloadModel(model);
    // UnloadTexture(texture);
    // UnloadImage(heightImage);
    // CloseWindow();
    // return 0;
}

//
// constexpr float CHUNK_SIZE_XZ = 24000.0f;
// constexpr float CHUNK_HEIGHT = 36000.0f;
// constexpr int GRID_MAX = 16;
// constexpr int VIEW_RADIUS = 4;
// constexpr float CAMERA_TILT_DOWN = 0.45f;
//
// struct TerrainChunk {
//     Model model{};
//     Texture2D texture{};
//     Vector3 drawPosition{};
//     bool active{};
//     std::vector<std::vector<float> > heightData;
// };
//
// /**
//  * Generate texture from height map image (grayscale)
//  * @param heightmap
//  * @return
//  */
// Texture2D GenerateHeightmapTexture(const Image &heightmap) {
//     Image image = GenImageColor(heightmap.width, heightmap.height, BLACK);
//     for (int y = 0; y < heightmap.height; y++) {
//         for (int x = 0; x < heightmap.width; x++) {
//             const Color pixel = GetImageColor(heightmap, x, y);
//             const float heightNormalized = static_cast<float>(pixel.r) / 255.0f;
//             Color color;
//             if (heightNormalized < 0.3f)
//                 color = (Color){30, 80, 180, 255};
//             else if (heightNormalized < 0.5f)
//                 color = (Color){210, 190, 150, 255};
//             else if (heightNormalized < 0.55f)
//                 color = (Color){200, 180, 140, 255};
//             else if (heightNormalized < 0.6f)
//                 color = (Color){40, 140, 50, 255};
//             else
//                 color = (Color){90, 80, 70, 255};
//             ImageDrawPixel(&image, x, y, color);
//         }
//     }
//     const Texture2D texture = LoadTextureFromImage(image);
//     UnloadImage(image);
//     return texture;
// }
//
// /**
//  * Load chunk of the map
//  * @param chunkX
//  * @param chunkZ
//  * @param fogShader
//  * @return
//  */
// TerrainChunk LoadTerrainChunk(int chunkX, int chunkZ, Shader fogShader) {
//     TerrainChunk chunk;
//     chunk.active = true;
//
//     const char *filename = TextFormat("terrain_chunks/chunk_%02d_%02d.png", chunkX, chunkZ);
//     Image image = LoadImage(filename);
//     if (image.data == nullptr) image = GenImageColor(256, 256, BLACK);
//
//     const char *textureFile = TextFormat("texture_chunks/chunk_%02d_%02d.png",chunkX, chunkZ);
//     Image textureImage = LoadImage(textureFile);
//     if (textureImage.data == nullptr) textureImage = GenImageColor(256, 256, BLACK);
//
//     const Texture2D texture = LoadTextureFromImage(textureImage);
//     UnloadImage(textureImage);
//
//     // allocate memory for the vector
//     chunk.heightData.resize(image.width, std::vector<float>(image.height));
//     for (int y = 0; y < image.height; y++) {
//         for (int x = 0; x < image.width; x++) {
//             Color c = GetImageColor(image, x, y);
//             chunk.heightData[x][y] = static_cast<float>(c.r) / 255.0f;
//         }
//     }
//
//     Vector3 size = {CHUNK_SIZE_XZ, CHUNK_HEIGHT, CHUNK_SIZE_XZ};
//     Mesh mesh = GenMeshHeightmap(image, size);
//     chunk.model = LoadModelFromMesh(mesh);
//
//     // החלת השיידר שיצרנו על המודל של האריח
//     chunk.model.materials[0].shader = fogShader;
//
//     chunk.texture = texture; // GenerateHeightmapTexture(image);
//     chunk.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = chunk.texture;
//
//     float posX = (static_cast<float>(chunkX) * CHUNK_SIZE_XZ) + (CHUNK_SIZE_XZ / 2.0f);
//     float posZ = (static_cast<float>(chunkZ) * CHUNK_SIZE_XZ) + (CHUNK_SIZE_XZ / 2.0f);
//     chunk.drawPosition = (Vector3){posX, 0.0f, posZ};
//
//     UnloadImage(image);
//     return chunk;
// }
//
// void ResetCamera(Camera3D &camera) {
// }

//
//
// int main() {
//     InitWindow(1280, 720, "Terrain");
//
//     // loading shader, the first missing param is the default vertex shader of raylib
//     // Shader fogShader = LoadShaderFromMemory(nullptr, fogShaderCode);
//
//     // mapping x,y location to list of active chunks to display
//     // std::map<std::pair<int, int>, TerrainChunk> activeChunks;
//     Camera3D camera = {0};
//     camera.up = (Vector3){0.0f, 1.0f, 0.0f};
//     camera.fovy = 85.0f;
//     camera.projection = CAMERA_PERSPECTIVE;
//
//     camera.position = (Vector3){0.0f, 100.0f, 0.0f};
//     camera.target = (Vector3){10.0f, 100.0f, 10.0f};
//
//     Vector3 forward = {0, 0, 1};
//     Vector3 up = {0, 1, 0};
//     float currentSpeed = 0.f;
//     // bool crashed = false;
//
//     SetTargetFPS(60);
//     rlSetClipPlanes(1.0f, 100.0f);
//
//
//     Image textureImage = LoadImage("texture.jpg");
//     const Texture2D texture = LoadTextureFromImage(textureImage);
//     UnloadImage(textureImage);
//
//     Image heightImage = LoadImage("heightmap.png");
//     // Vector3 size = {10000.0f, 10000.0f, 10000.0f};
//     Vector3 size = {100.0f, 100.0f, 100.0f};
//     Mesh mesh = GenMeshHeightmap(heightImage, size);
//     UnloadImage(heightImage);
//
//     Model model = LoadModelFromMesh(mesh);
//
//     model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
//
//     Image cockpit = LoadImage("cockpit.png");
//     Texture2D cockpitTexture = LoadTextureFromImage(cockpit);
//
//     while (!WindowShouldClose()) {
//         float deltaTime = GetFrameTime();
//
//         // if (crashed) {
//         //     BeginDrawing();
//         //     ClearBackground(MAROON);
//         //     DrawText("CRASHED!", 1280 / 2 - 100, 720 / 2, 50, WHITE);
//         //     EndDrawing();
//         //     continue;
//         // }
//
//         // flight
//         float pitch = 0.0f;
//         float roll = 0.0f;
//         float yaw = 0.0f;
//         if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) pitch = -1.0f * deltaTime;
//         if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) pitch = 1.0f * deltaTime;
//         if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) roll = -2.0f * deltaTime;
//         if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) roll = 2.0f * deltaTime;
//         if (IsKeyDown(KEY_Q)) yaw = 0.5f * deltaTime;
//         if (IsKeyDown(KEY_E)) yaw = -0.5f * deltaTime;
//
//         if (IsKeyDown(KEY_EQUAL) || IsKeyDown(KEY_TWO)) currentSpeed += 10.0f * deltaTime;
//         if (IsKeyDown(KEY_MINUS) || IsKeyDown(KEY_ONE)) currentSpeed -= 10.0f * deltaTime;
//         // if (currentSpeed < 50.0f) currentSpeed = 50.0f;
//         if (currentSpeed > 4000.0f) currentSpeed = 4000.0f;
//
//         // reset nose
//         if (IsKeyDown(KEY_R)) up = {0, 1, 0};
//
//         // find right normal
//         Vector3 right = Vector3CrossProduct(forward, up);
//         // calculate the forward using the right normal and pitch
//         forward = Vector3RotateByAxisAngle(forward, right, pitch);
//         // calculate the up pitch
//         up = Vector3RotateByAxisAngle(up, right, pitch);
//         // calculate the app roll
//         up = Vector3RotateByAxisAngle(up, forward, roll);
//
//         // update forward and up yaw (not part of the flight)
//         forward = Vector3RotateByAxisAngle(forward, (Vector3){0, 1, 0}, yaw);
//         up = Vector3RotateByAxisAngle(up, (Vector3){0, 1, 0}, yaw);
//
//         // camera.position = Vector3Add(camera.position, Vector3Scale(forward, currentSpeed * deltaTime));
//         // camera.target = Vector3Add(camera.position, forward);
//         // camera.up = up;
//
//         // 1. הזזת המטוס (הפיזיקה נשארת נאמנה לוקטור ה-forward האמיתי של הטיסה)
//         camera.position = Vector3Add(camera.position, Vector3Scale(forward, currentSpeed * deltaTime));
//
//         // 2. חישוב זווית הראייה של הטייס (מוטה למטה ביחס לאף המטוס)
//         // אנחנו מסובבים את וקטור ה"קדימה" ווקטור ה"למעלה" סביב ציר ה"ימינה" (הכנפיים)
//         Vector3 pilotLookDir = Vector3RotateByAxisAngle(forward, right, -CAMERA_TILT_DOWN);
//         Vector3 pilotUpDir = Vector3RotateByAxisAngle(up, right, -CAMERA_TILT_DOWN);
//
//         // 3. עדכון המצלמה לפי וקטור המבט החדש
//         camera.target = Vector3Add(camera.position, pilotLookDir);
//         camera.up = pilotUpDir;
//
//
//         // --- הסטת חלון הטעינה (Directional Streaming) ---
//         // int currentChunkX = static_cast<int>(floor(camera.position.x / CHUNK_SIZE_XZ));
//         // int currentChunkZ = static_cast<int>(floor(camera.position.z / CHUNK_SIZE_XZ));
//
//
//         // סימון הכל כישן
//         // for (auto &pair: activeChunks) pair.second.active = false;
//
//         // for (int z = currentChunkZ - VIEW_RADIUS; z <= currentChunkZ + VIEW_RADIUS; z++) {
//         //     for (int x = currentChunkX - VIEW_RADIUS; x <= currentChunkX + VIEW_RADIUS; x++) {
//         //         if (x >= 0 && x < GRID_MAX && z >= 0 && z < GRID_MAX) {
//         //             auto key = std::make_pair(x, z);
//         //             if (activeChunks.find(key) == activeChunks.end()) {
//         //                 activeChunks[key] = LoadTerrainChunk(x, z, fogShader);
//         //             }
//         //             activeChunks[key].active = true;
//         //         }
//         //     }
//         // }
//
//         // מחיקת ישנים (כאן אנחנו חייבים להשתמש באיטרטור, לכן נשאר pair.second)
//         // for (auto it = activeChunks.begin(); it != activeChunks.end();) {
//         //     if (!it->second.active) {
//         //         UnloadModel(it->second.model);
//         //         UnloadTexture(it->second.texture);
//         //         it = activeChunks.erase(it);
//         //     } else ++it;
//         // }
//
//         // התרסקות
//         // float groundHeight = -100.0f;
//         // auto currentKey = std::make_pair(currentChunkX, currentChunkZ);
//         // if (activeChunks.find(currentKey) != activeChunks.end()) {
//         //     float localX = fmod(camera.position.x, CHUNK_SIZE_XZ);
//         //     float localZ = fmod(camera.position.z, CHUNK_SIZE_XZ);
//         //     int pixelX = static_cast<int>((localX / CHUNK_SIZE_XZ) * 255.0f);
//         //     int pixelZ = static_cast<int>((localZ / CHUNK_SIZE_XZ) * 255.0f);
//         //     if (pixelX >= 0 && pixelX < 256 && pixelZ >= 0 && pixelZ < 256) {
//         //         groundHeight = activeChunks[currentKey].heightData[pixelX][pixelZ] * CHUNK_HEIGHT;
//         //     }
//         // }
//         // if (camera.position.y <= groundHeight + 5.0f) crashed = true;
//
//         // --- ציור ---
//         BeginDrawing();
//         ClearBackground((Color){135, 206, 235, 255}); // תכלת שמתאים לערפל שלנו
//
//         BeginMode3D(camera);
//         // ** שיפור C++ מודרני: קריא וברור, ללא pair.second! **
//         // for (const auto &[coords, chunk]: activeChunks) {
//         //     DrawModel(chunk.model, chunk.drawPosition, 1.0f, WHITE);
//         // }
//         DrawModel(model, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
//         DrawGrid(100, 10.0f);
//         EndMode3D();
//
//         DrawText(TextFormat("Cx: %0.0f", camera.position.x), 20, 20, 20, LIME);
//         DrawText(TextFormat("Cy: %0.0f", camera.position.y), 20, 40, 20, LIME);
//         DrawText(TextFormat("Cz: %0.0f", camera.position.z), 20, 60, 20, LIME);
//
//         DrawText(TextFormat("Tx: %0.0f", camera.target.x), 20, 80, 20, YELLOW);
//         DrawText(TextFormat("Ty: %0.0f", camera.target.y), 20, 100, 20, YELLOW);
//         DrawText(TextFormat("Tz: %0.0f", camera.target.z), 20, 120, 20, YELLOW);
//
//         DrawTexture(cockpitTexture, 0, 100, WHITE);
//
//         // DrawText(TextFormat("ALT G: %0.0f", groundHeight), 20, 40, 20, LIME);
//         // DrawText(TextFormat("SPD: %0.0f", currentSpeed), 20, 60, 20, LIME);
//         // DrawText(TextFormat("SECTOR: X:%d Z:%d", currentChunkX, currentChunkZ), 20, 80, 20, YELLOW);
//         // DrawText(TextFormat("ACTIVE CHUNKS: %zu", activeChunks.size()), 20, 100, 20, ORANGE);
//
//         DrawLine(1280 / 2 - 15, 720 / 2, 1280 / 2 + 15, 720 / 2, BLACK);
//         DrawLine(1280 / 2, 720 / 2 - 15, 1280 / 2, 720 / 2 + 15, BLACK);
//
//         EndDrawing();
//     }
//
//     // ניקוי ביציאה
//     // for (auto &pair: activeChunks) {
//     //     UnloadModel(pair.second.model);
//     //     UnloadTexture(pair.second.texture);
//     // }
//     // UnloadShader(fogShader);
//     UnloadModel(model);
//     UnloadTexture(texture);
//     UnloadImage(heightImage);
//     CloseWindow();
//     return 0;
// }
