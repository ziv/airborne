#pragma once
#include "Entity.h"
#include "../core/AircraftStructs.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

class EntityRegistry {
    std::unordered_map<EntityId, size_t> idIndex;
    std::vector<std::unique_ptr<EntityBase>> entities;
    std::unordered_map<std::string, Model> modelCache;

    void ensureModelLoaded(const std::string& path);

public:
    EntityRegistry() = default;
    ~EntityRegistry();

    EntityRegistry(const EntityRegistry&) = delete;
    EntityRegistry& operator=(const EntityRegistry&) = delete;
    EntityRegistry(EntityRegistry&&) = delete;
    EntityRegistry& operator=(EntityRegistry&&) = delete;

    EntityBase* spawn(std::unique_ptr<EntityBase> entity);
    void destroy(const EntityId& id);
    void despawn(const EntityId& id);

    EntityBase* get(const EntityId& id);
    [[nodiscard]] const EntityBase* get(const EntityId& id) const;
    std::vector<EntityBase*> getByType(EntityType type);
    std::vector<EntityBase*> getByFaction(Faction faction);
    std::vector<EntityBase*> getInRadius(Vector3 center, float radius);

    /// Read-only access to the full entity list (for iteration without mutation).
    [[nodiscard]] const std::vector<std::unique_ptr<EntityBase>>& allEntities() const { return entities; }

    void forEach(const std::function<void(EntityBase&)>& fn);
    void forEachAlive(const std::function<void(EntityBase&)>& fn);

    void update(const AircraftState& playerState, float dt);
    void draw(const AircraftState& playerState) const;
};
