#pragma once
#include <future>
#include "AircraftStructs.h"
#include "../primitives/AppConfig.h"
#include "../primitives/Resource.h"

struct AsyncChunkResult {
    Image heightImage;
    Image textureImage;
};

struct PendingChunk {
    int x, z;
    std::future<AsyncChunkResult> futureData; // הבטחה לקבל Mesh בעתיד
};

struct CloudInfo {
    Vector3 position;
    float size;
};

class SceneManager {
    std::string txTemplate = "res/tx16k/tx-%i-%i.png";
    std::string hmTemplate = "res/hm1616k/hm-%i-%i.png";
    MusicHandle engineSound;
    ImageHandle height;
    ModelHandle map;
    float relativeHeight;
    ShaderHandle fog;

    // cloud layer
    ModelHandle cloudModel;
    TextureHandle cloudTexture;

public:
    explicit SceneManager(const AppConfig &config);

    // ~SceneManager();

    void update(AircraftState &state, float dt);

    void draw(const AircraftState &state, const Camera &camera);
};
