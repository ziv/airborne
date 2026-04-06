#pragma once
#include "raylib.h"
#include "../core/AircraftStructs.h"

class RadarView {
    Vector2 radarCenter = Vector2{373.0, 684.0};
    float currentRadarRange = 1000;

    // Vector3 playerPos;
    // Vector3 playerForward;
    // Vector3 enemyPos;
public:

    void update();
    void draw(const AircraftState &state, Vector3 enemy);
};
