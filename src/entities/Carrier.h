#pragma once
#include "Entity.h"

class Carrier : public EntityBase {
    float heightAboveGround = 8.0f;
    float landingBoxWidth = 200.0f;
    float landingBoxLength = 500.0f;
    float landingBoxHeight = 100.0f;
    float radiusSquare = 250.0 * 250.0;

public:
    using EntityBase::EntityBase;

    void draw(const AircraftState &state) override;

    void update(AircraftState &state, float dt) override;
};
