#pragma once
#include "raylib.h"
#include "../core/AircraftControls.h"

class DebugView {
public:
    void draw(const Vector3 &position, const PilotControls &controls, float speed);
};
