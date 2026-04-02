#pragma once
#include "raylib.h"
#include "../core/AircraftTransformation.h"
#include "../core/AppConfig.h"
#include "../primitives/Resource.h"

class NavballView {
    TextureHandle tex;
    Mesh sphere;
    Model navball;

public:
    explicit NavballView(const AppConfig &config);

    void draw(const Vector3 &position, const Directions &directions);
};
