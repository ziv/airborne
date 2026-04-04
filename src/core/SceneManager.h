#pragma once
#include "AircraftStructs.h"
#include "../primitives/AppConfig.h"
#include "../primitives/Resource.h"
//
// struct TerrainChunk {
//     ModelHandle model;
//     Vector3 position;
//     bool isVisible;
//
//     TerrainChunk(const Model &model, const Vector3 pos) : model(model),
//                                                           position(pos),
//                                                           isVisible(true) {
//     }
// };

class SceneManager {
    std::string txTemplate = "res/16kparts/tx-%i-%i.png";
    std::string hmTemplate = "res/16kparts/hm-%i-%i.png";
    MusicHandle engineSound;
    Image height;
    ModelHandle map;
    float relativeHeight;

    // ניהול זיכרון בטוח לשיידר
    // Shader fogShader;

    // מיקומים (Locations) של משתנים בתוך השיידר
    // int camPosLoc;
    // int fogColorLoc;
    // int fogDensityLoc;

    // הגדרות ערפל
    // Color CurrentFogColor = { 200, 210, 220, 255 }; // אפור-כחול שמיים
    // float CurrentFogDensity = 0.00012f;            // ערך נמוך למרחקים עצומים

    // std::vector<TerrainChunk> chunks;
    // std::map<std::pair<int, int>, ModelHandle> activeChunks;

    // const float CHUNK_SIZE = 2048.0f;
    // const int GRID_LIMIT = 16;


    // void loadChunk(int x, int z);
    // void addChunk(Model rawModel, Vector3 position);

public:
    explicit SceneManager(const AppConfig &config);

    ~SceneManager();

    void update(AircraftState &state, float dt);

    void draw() const;

    // [[nodiscard]] float getHeight(int x, int z) const;
};
