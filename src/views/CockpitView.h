#pragma once
#include "../core/AircraftStructs.h"
#include "../primitives/AppConfig.h"
#include "../primitives/Resource.h"

class CockpitView {
    TextureHandle cockpitTexture;
    ShaderHandle cockpitShader;
    TextureHandle sprites;

public:
    explicit CockpitView(const AppConfig &config);

    void draw(const AircraftState &state) const;
};
