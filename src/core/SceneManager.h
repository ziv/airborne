#pragma once
#include "AircraftStructs.h"
#include "../primitives/AppConfig.h"
#include "../primitives/Resource.h"

class SceneManager {
    MusicHandle engineSound;
    Image height;
    ModelHandle map;
    float relativeHeight;

public:
    explicit SceneManager(const AppConfig &config);

    void update(const AircraftState &state, float dt);

    void draw(float dt);

    [[nodiscard]] float getHeight(int x, int z) const;
};
