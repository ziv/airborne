/**
 * @file EntityRegistry.cpp
 * @brief Entity lifecycle, model caching, and 3D rendering (with landing-zone visualisation).
 */
#include "EntityRegistry.h"
#include "raymath.h"
#include "rlgl.h"
#include <cstdio>

EntityRegistry::~EntityRegistry() {
    for (auto &[path, model]: modelCache) {
        UnloadModel(model);
    }
    modelCache.clear();
}

void EntityRegistry::ensureModelLoaded(const std::string &path) {
    if (path.empty()) return;
    if (modelCache.contains(path)) return;

    if (!FileExists(path.c_str())) {
        TraceLog(LOG_WARNING, "Model file not found: %s", path.c_str());
        return;
    }
    modelCache[path] = LoadModel(path.c_str());
    TraceLog(LOG_INFO, "Model loaded: %s", path.c_str());
}

EntityBase *EntityRegistry::spawn(std::unique_ptr<EntityBase> entity) {
    const auto &id = entity->id;
    ensureModelLoaded(entity->modelId);
    const size_t index = entities.size();
    entities.push_back(std::move(entity));
    idIndex[id] = index;
    TraceLog(LOG_INFO, "Entity spawned: %s", id.c_str());
    return entities.back().get();
}

void EntityRegistry::destroy(const EntityId &id) {
    if (auto *e = get(id)) {
        e->state = EntityState::DESTROYED;
    }
}

void EntityRegistry::despawn(const EntityId &id) {
    if (auto *e = get(id)) {
        e->state = EntityState::DESPAWNED;
    }
}

EntityBase *EntityRegistry::get(const EntityId &id) {
    const auto it = idIndex.find(id);
    if (it == idIndex.end()) return nullptr;
    return entities[it->second].get();
}

const EntityBase *EntityRegistry::get(const EntityId &id) const {
    const auto it = idIndex.find(id);
    if (it == idIndex.end()) return nullptr;
    return entities[it->second].get();
}

std::vector<EntityBase *> EntityRegistry::getByType(EntityType type) {
    std::vector<EntityBase *> result;
    for (auto &e: entities) {
        if (e->type == type && e->isAlive()) result.push_back(e.get());
    }
    return result;
}

std::vector<EntityBase *> EntityRegistry::getByFaction(Faction faction) {
    std::vector<EntityBase *> result;
    for (auto &e: entities) {
        if (e->faction == faction && e->isAlive()) result.push_back(e.get());
    }
    return result;
}

std::vector<EntityBase *> EntityRegistry::getInRadius(Vector3 center, float radius) {
    std::vector<EntityBase *> result;
    const float radiusSq = radius * radius;
    for (auto &e: entities) {
        if (!e->isAlive()) continue;
        const Vector3 diff = Vector3Subtract(e->position, center);
        if (Vector3LengthSqr(diff) <= radiusSq) {
            result.push_back(e.get());
        }
    }
    return result;
}

void EntityRegistry::update(const AircraftState & /*playerState*/, float /*dt*/) {
    // Phase 1: static entities only — no AI update needed
}

void EntityRegistry::draw(const AircraftState &playerState) const {
    for (const auto &e: entities) {
        if (!e->isAlive()) continue;

        // convert entity world position into the camera's local coordinate frame
        const Vector3 drawPos = {
            e->position.x + playerState.mapOffset.x,
            e->position.y,
            e->position.z + playerState.mapOffset.y
        };

        if (playerState.tooFar2Draw(drawPos)) continue;

        // --- draw the entity model (or fallback cube) ---
        auto it = modelCache.find(e->modelId);
        if (it != modelCache.end()) {
            DrawModelEx(it->second, drawPos, {0, 1, 0}, e->heading, {1.0f, 1.0f, 1.0f}, WHITE);

            // debug wireframe outline
            Color color;
            float size;
            switch (e->faction) {
                case Faction::ENEMY: color = RED;
                    break;
                case Faction::FRIENDLY: color = GREEN;
                    break;
                default: color = GRAY;
                    break;
            }
            switch (e->type) {
                case EntityType::STRUCTURE: size = 50.0f;
                    break;
                case EntityType::AIRBASE: size = 100.0f;
                    break;
                default: size = 30.0f;
                    break;
            }
            // DrawCube(drawPos, size, size, size, color);
            DrawCubeWires(drawPos, size, size, size, BLACK);
        } else {
            // fallback cube when no model loaded
            Color color;
            float size;
            switch (e->faction) {
                case Faction::ENEMY: color = RED;
                    break;
                case Faction::FRIENDLY: color = GREEN;
                    break;
                default: color = GRAY;
                    break;
            }
            switch (e->type) {
                case EntityType::STRUCTURE: size = 50.0f;
                    break;
                case EntityType::AIRBASE: size = 100.0f;
                    break;
                default: size = 30.0f;
                    break;
            }
            DrawCube(drawPos, size, size, size, color);
            DrawCubeWires(drawPos, size, size, size, BLACK);
        }

        // --- draw 3D landing-zone box for AIRBASE entities ---
        // A semi-transparent wireframe cuboid marks the valid approach volume.
        // Entering this box with gear down, level wings, and low speed = safe landing.
        if (e->type == EntityType::AIRBASE) {
            const bool isCarrier = (e->subtype == "carrier");

            // dimensions: short side (width) across the heading, long side along it
            const float stripWidth = 400.0f; // isCarrier ? 400.0f : 800.0f;
            const float stripLength = 2000.0f; // isCarrier ? 2000.0f : 4000.0f;
            const float boxHeight = 150.0f; // must match GameData::LANDING_BOX_HEIGHT

            // box sits on the landing surface
            const float surfaceY = isCarrier ? 150.0f : 0.0f;
            // center the box vertically: bottom at surfaceY, top at surfaceY + boxHeight
            const float boxCenterY = surfaceY + boxHeight * 0.5f;

            // apply heading rotation, then draw a thin cuboid
            rlPushMatrix();
            rlTranslatef(drawPos.x, boxCenterY, drawPos.z);
            rlRotatef(e->heading, 0, 1, 0);
            // X = width (across heading), Y = box height, Z = length (along heading)
            DrawCube({0, 0, 0}, stripWidth, boxHeight, stripLength, Fade(GREEN, 0.08f));
            DrawCubeWires({0, 0, 0}, stripWidth, boxHeight, stripLength, GREEN);
            rlPopMatrix();
        }
    }
}
