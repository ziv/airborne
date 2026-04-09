#pragma once
#include "../../core/AircraftStructs.h"

class EngineView {

    void drawEngine(const AircraftState &state, const Vector2 &center) const;
public:
    void draw(const AircraftState &state, const Vector2 &position) const;
};
