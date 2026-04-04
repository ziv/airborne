#pragma once
#include "raylib.h"
#include "../core/AircraftTransformation.h"
#include "../primitives/AppConfig.h"
#include "../primitives/Resource.h"

class NavballView {
    TextureHandle tex;
    Mesh sphere;
    Model navball;

    // virtual studio
    // RenderTextureHandle vtex;
    RenderTexture2D vtex;
    Camera vcam = {0};

public:
    explicit NavballView(const AppConfig &config);

    void draw(const AircraftState &state);
};
