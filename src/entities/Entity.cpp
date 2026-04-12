//
// Created by Ziv Perry on 12/04/2026.
//
#include "Entity.h"

void EntityBase::draw(const AircraftState &state) {
    if (!et.isAlive()) return;

    // todo draw the model if modelId is set, otherwise fallback to a cube
    DrawCube(et.position, 20 * et.scale, 20 * et.scale, 20 * et.scale, et.isEnemy() ? RED : GREEN);
}
