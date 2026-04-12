#include "Airbase.h"
#include "EntitiesUtils.h"
#include "rlgl.h"
#include "../primitives/Math.h"


void Airbase::draw(const AircraftState &state) {
    EntityBase::draw(state);
    // landing box

    // this version also rotate by heading
    const auto pos = position(state);
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlRotatef(def.heading, 0.0f, 1.0f, 0.0f);
    DrawCubeWires({0.0f, 0.0f, 0.0f}, landingBoxLength, landingBoxHeight, landingBoxWidth, YELLOW);
    rlPopMatrix();
    // DrawCubeWires(position(state), landingBoxLength, landingBoxHeight, landingBoxWidth, YELLOW);
}

void Airbase::update(AircraftState &state, float dt) {
    // EntityBase::update(state, dt);
    const auto pos = position(state);
    const auto aircraftPos = state.pos();

    if (SquareDistance(pos, aircraftPos) < radiusSquare) {
        if (AircraftInLandingBox(pos, aircraftPos, landingBoxLength, landingBoxWidth, def.heading)) {
            state.landingZone = {true, false, 0.0f};
        }
    }
}
