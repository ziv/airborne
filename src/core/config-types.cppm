module;
#include <nlohmann/json.hpp>
#include <string>
#include "../lib/ray.hpp"

export module Types:Config;

import :Units;

export struct GlobalConfig {
    // window
    std::string title;
    Pixel width;
    Pixel height;

    // player tilt down and field of view
    AngleRad tilt;
    AngleDeg fov;

    // where to clip planes
    Meter nearPlane;
    Meter farPlane;
};

export struct WindowConfig {
    std::string title;
    Pixel width;
    Pixel height;
    Meter nearPlane;
    Meter farPlane;
};

export struct SceneConfig {
    std::string mapTexture;
    std::string mapHeightmap;
    Vector3 mapSize;
    std::string fogShaderVs;
    std::string fogShaderFs;
};

export struct CockpitConfig {
    std::string texturePath;
    std::string shaderPath;
    Color tintColor{};
};

export struct MinimapConfig {
    std::string mapTexture; ///< Path to the satellite map texture.
    Pixel size = 150; ///< Size (width and height) of the square minimap widget.
    Ratio mapsRatio = 62.5f; ///< Ratio between the world coordinates and the map texture coordinates
};

// HUD config from here

export struct HudLadderConfig {
    Pixel x = 440;
    Pixel y = 200;
    Pixel width = 320;
    Pixel height = 220;
    Pixel offset = 0;
};

export struct RocConfig {
    Pixel x;
    Pixel y;
    Pixel width;
    Pixel height;
};

export struct SpeedometerConfig {
    Pixel x;
    Pixel y;
    Size font;
};

export struct HeightIndicatorConfig {
    Pixel x;
    Pixel y;
    Size font;
};

export struct BoresightConfig {
    Pixel x;
    Pixel y;
    Size size;
};


export struct HeadingConfig {
    Pixel x;
    Pixel y;
    Pixel width;
    Size font;
};

export struct WarningsConfig {
    Pixel x;
    Pixel y;
    Size font;
};

export struct HudConfig {
    HudLadderConfig ladder;
    RocConfig roc;
    SpeedometerConfig speedometer;
    HeightIndicatorConfig height;
    BoresightConfig boresight;
    HeadingConfig heading;
    WarningsConfig warnings;
};

export struct RadarConfig {
    Pixel size;
    std::array<Meter, 3> ranges;
};

export
{
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GlobalConfig, title, width, height, tilt, fov, nearPlane, farPlane);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WindowConfig, title, width, height, nearPlane, farPlane);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SceneConfig, mapTexture, mapHeightmap, mapSize, fogShaderVs, fogShaderFs);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CockpitConfig, texturePath, shaderPath, tintColor);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MinimapConfig, mapTexture, size, mapsRatio);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HudLadderConfig, x, y, width, height, offset);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RocConfig, x, y, width, height);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpeedometerConfig, x, y, font);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HeightIndicatorConfig, x, y, font);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BoresightConfig, x, y, size);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HeadingConfig, x, y, width, font);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WarningsConfig, x, y, font);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HudConfig, ladder, roc, speedometer, height, boresight, heading, warnings);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RadarConfig, size, ranges);
}
