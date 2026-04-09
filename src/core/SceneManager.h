/**
 * @file SceneManager.h
 * @brief Manages the static 3D scene: terrain mesh, fog shader, cloud layer,
 *        and engine audio.
 *
 * Reads terrain heightmap pixels on the CPU each frame to supply ground-height
 * data to the physics system. Renders the terrain and cloud plane in 3D.
 */
#pragma once
#include <future>
#include "AircraftStructs.h"
#include "../primitives/AppConfig.h"
#include "../primitives/Resource.h"

/// @brief Intermediate result from an async terrain chunk load (unused in current single-map path).
struct AsyncChunkResult {
    Image heightImage;
    Image textureImage;
};

/// @brief A terrain chunk whose images are being loaded asynchronously.
struct PendingChunk {
    int x, z;
    std::future<AsyncChunkResult> futureData;
};

/// @brief Metadata for a single cloud billboard (reserved for future procedural clouds).
struct CloudInfo {
    Vector3 position;
    float size;
};

/**
 * @brief Owns the terrain model, fog shader, cloud layer, and engine sound.
 *
 * On construction it loads the full-map heightmap, texture, and fog shader.
 * Each frame it samples the heightmap to update AircraftState::groundHeight
 * and modulates the engine sound pitch/volume from the throttle setting.
 */
class SceneManager {
    std::string txTemplate = "res/tx16k/tx-%i-%i.png";
    std::string hmTemplate = "res/hm1616k/hm-%i-%i.png";
    MusicHandle engineSound;   ///< Looping engine sound whose pitch tracks throttle.
    ImageHandle height;        ///< CPU-side heightmap image used for ground-height sampling.
    ModelHandle map;           ///< GPU terrain model (heightmap mesh + satellite texture).
    float relativeHeight;      ///< Maximum terrain height in meters (Y scale of the heightmap).
    ShaderHandle fog;          ///< Distance-fog fragment shader applied to terrain and clouds.

    ModelHandle cloudModel;    ///< Large flat plane for the cloud layer.
    TextureHandle cloudTexture;///< Perlin-noise-based cloud alpha texture.

public:
    explicit SceneManager(const AppConfig &config);

    /// @brief Update ground height, engine audio, and cloud animation.
    /// @param state  Aircraft state — groundHeight is written, position is read.
    /// @param dt     Frame delta time (seconds).
    void update(AircraftState &state, float dt);

    /// @brief Draw the terrain and cloud layer in 3D.
    void draw(const AircraftState &state, const Camera &camera);
};
