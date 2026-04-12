//
// Created by Ziv Perry on 12/04/2026.
//
#include "Entity.h"

#include "EntityRegistry.h"

Vector3 EntityBase::position(const AircraftState &state) const {
    return (Vector3){
        def.position.x + state.mapOffset.x,
        def.position.y,
        def.position.z + state.mapOffset.y
    };
}

void EntityBase::draw(const AircraftState &state) {
    if (!def.isAlive()) return;
    const Vector3 drawPos = {
        def.position.x + state.mapOffset.x,
        def.position.y,
        def.position.z + state.mapOffset.y
    };

    if (state.tooFar2Draw(drawPos)) return;
    if (EntityRegistry::get().modelCache.contains(def.modelId)) {
        DrawModelEx(EntityRegistry::get().modelCache[def.modelId], drawPos, {0, 1, 0}, def.heading, {1.0f, 1.0f, 1.0f}, WHITE);
    } else {
        DrawCube(drawPos, 100 * def.scale, 100 * def.scale, 100 * def.scale, def.isEnemy() ? RED : GREEN);
    }
}
