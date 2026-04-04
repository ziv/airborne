#pragma once
#include "../primitives/AppConfig.h"
#include "../primitives/Resource.h"

class CockpitView {
    TextureHandle cockpitTexture;
    ShaderHandle cockpitShader;

public:
    explicit CockpitView(const AppConfig &config);

    void draw() const;
};
