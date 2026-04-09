/**
 * @file EntityRegistry.h
 * @brief Central entity container — owns all spawned entities and handles
 *        lookup, lifecycle, 3D model caching, and rendering.
 */
#pragma once
#include "Entity.h"
#include "../core/AircraftStructs.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <concepts>

/**
 * @brief Manages the lifetime and rendering of all game entities.
 *
 * Entities are stored as unique_ptr<EntityBase> and indexed by their string ID.
 * 3D models are loaded on demand and cached so duplicates share a single GPU
 * resource. The registry is non-copyable and non-movable.
 */
class EntityRegistry {
    std::unordered_map<EntityId, size_t> idIndex; ///< Entity ID → vector index.
    std::vector<std::unique_ptr<EntityBase> > entities; ///< Owning storage for all entities.
    std::unordered_map<std::string, Model> modelCache; ///< Loaded 3D models keyed by file path.

    /// @brief Load a model from disk if not already in the cache.
    void ensureModelLoaded(const std::string &path);

public:
    EntityRegistry() = default;

    ~EntityRegistry();

    EntityRegistry(const EntityRegistry &) = delete;

    EntityRegistry &operator=(const EntityRegistry &) = delete;

    EntityRegistry(EntityRegistry &&) = delete;

    EntityRegistry &operator=(EntityRegistry &&) = delete;

    /// @brief Add a new entity to the registry and load its model.
    /// @return Raw pointer to the spawned entity (registry retains ownership).
    EntityBase *spawn(std::unique_ptr<EntityBase> entity);

    /// @brief Mark an entity as DESTROYED (remains in registry for scoring).
    void destroy(const EntityId &id);

    /// @brief Mark an entity as DESPAWNED (logically removed from the world).
    void despawn(const EntityId &id);

    /// @brief Look up an entity by its unique ID. Returns nullptr if not found.
    EntityBase *get(const EntityId &id);

    [[nodiscard]] const EntityBase *get(const EntityId &id) const;

    /// @brief Return all alive entities of a given type.
    std::vector<EntityBase *> getByType(EntityType type);

    /// @brief Return all alive entities belonging to a faction.
    std::vector<EntityBase *> getByFaction(Faction faction);

    /// @brief Return all alive entities within @p radius meters of @p center.
    std::vector<EntityBase *> getInRadius(Vector3 center, float radius);

    /// Read-only access to the full entity list (for iteration without mutation).
    [[nodiscard]] const std::vector<std::unique_ptr<EntityBase> > &allEntities() const { return entities; }

    /// @brief Invoke @p fn on every entity (including dead/despawned).
    /// this is template, notice the auto
    void forEach(std::invocable<EntityBase&> auto fn) {
        for (auto &e: entities) {
            fn(*e);
        }
    }

    /// @brief Invoke @p fn only on entities in ACTIVE or DAMAGED state.
    /// this is template, notice the auto
    void forEachAlive(std::invocable<EntityBase&> auto fn) {
        for (auto &e: entities) {
            if (e->isAlive()) {
                fn(*e);
            }
        }
    }

    /// @brief Per-frame entity logic (currently a no-op for static entities).
    void update(const AircraftState &playerState, float dt);

    /// @brief Draw all alive entities with their models (or fallback cubes) and landing-zone boxes.
    void draw(const AircraftState &playerState) const;
};
