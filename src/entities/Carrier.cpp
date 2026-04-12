//
// Created by Ziv Perry on 12/04/2026.
//

#include "Carrier.h"

#include "EntitiesUtils.h"
#include "rlgl.h"
#include "../primitives/Math.h"

void Carrier::draw(const AircraftState &state) {
    EntityBase::draw(state);
    // landing box
    const auto pos = position(state);
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlRotatef(def.heading, 0.0f, 1.0f, 0.0f);
    DrawCubeWires({0.0f, 0.0f, 0.0f}, landingBoxHeight, landingBoxHeight, landingBoxLength, YELLOW);
    rlPopMatrix();
    // DrawCubeWires(position(state), landingBoxLength, landingBoxHeight, landingBoxWidth, YELLOW);
}

void Carrier::update(AircraftState &state, float dt) {
    // EntityBase::update(state, dt);
    const auto carrierPos = position(state);
    const auto aircraftPos = state.pos();

    if (SquareDistance(carrierPos, aircraftPos) < radiusSquare) {
        if (AircraftInLandingBox(carrierPos, aircraftPos, landingBoxLength, landingBoxWidth, def.heading)) {
            state.landingZone = {true, true, heightAboveGround};
        }
    }
}
