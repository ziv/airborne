#pragma once
#include <vector>
#include "raylib.h"
#include "../core/AircraftStructs.h"
#include "../primitives/Types.h"

struct RadarContact {
    Vector3 worldPosition{};
    Color color = RED;
};

class RadarView {
    // Vector2 center = {377.0f, 666.0f};
    float displayRadius = 60.0f;

    static constexpr Meter RANGES[] = {10000.0f, 40000.0f, 100000.0f};
    static constexpr int RANGE_COUNT = 3;
    int rangeIndex = 0;

    void drawScope(const Vector2 &center) const;

    // void drawContact(Vector2 blipPos, const RadarContact &contact) const;

public:
    void update();

    void draw(const AircraftState &state,
              const std::vector<RadarContact> &contacts,
              const Vector2 &center) const;
};
