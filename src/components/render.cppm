module;
#include <entt/entt.hpp>

export module Components:Render;

import RaylibResource;
import ResourceManager;
import Types;

// resources

export struct WithModel {
    entt::resource<ModelResourceLoader> handle;
};

export struct WithTexture {
    entt::resource<TextureResourceLoader> handle;
};

export struct WithImage {
    entt::resource<ImageResourceLoader> handle;
};

export struct WithFsShader {
    entt::resource<ShaderLoader> handle;
};

// screen slots

export struct DashboardSlot {
    int slot_index;
};


// widgets tags

export struct CockpitWidget {
};

export struct HudWidget {
    HudConfig cfg{};
    Pixel centerX;
    Pixel centerY;
    Pixel pixelsPerDegree;
    float ppd;
    float tilt;
    int colorIndex;
};

export struct RadarWidget {
    RadarConfig cfg{};
    int rangeCount = 0;
    int rangeIndex = 0;
};

export struct MinimapWidget {
    MinimapConfig cfg{};
    float zoom = 1.0f;
};

export struct EngineWidget {
};
