#pragma once
#include "Entity.h"

class Airbase : public EntityBase {
    float landingBoxWidth = 400.0f;
    float landingBoxLength = 1000.0f;
    float landingBoxHeight = 100.0f;
    float radiusSquare = 500.0 * 500.0;

    float minx;
    float maxx;
    float minz;
    float maxz;

public:
    explicit Airbase(const EntityDef &def) : EntityBase(def) {
        minx = def.getParam("minx");
        maxx = def.getParam("maxx");
        minz = def.getParam("minz");
        maxz = def.getParam("maxz");
    }

    void draw(const AircraftState &state) override;

    void update(AircraftState &state, float dt) override;
};
