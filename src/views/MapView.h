#pragma once
#include "raylib.h"
#include "../core/AircraftStructs.h"
#include "../primitives/AppConfig.h"
#include "../primitives/Resource.h"


class MapView {
    // configuration
    TextureHandle tex;
    ShaderHandle glass;
    int timeLoc;

    // state
    Camera2D mapCamera = {0};
    float heading = 0.0f;
    float zoom = 1.0;

public:
    explicit MapView(const AppConfig &config);

    void update(const AircraftState &state);

    void draw();
};
