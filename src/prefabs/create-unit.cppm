module;
#include <entt/entt.hpp>
#include "../lib/ray.hpp"

export module Prefabs:Unit;

import JsonConfig;
import Types;
import ResourceManager;
import Components;

export namespace factories {
    entt::entity create_unit(entt::registry &registry,
                            const EntityDef &def) {
        const auto entity = registry.create();

        registry.emplace<Identify>(entity, def.id);
        registry.emplace<IdentifyType>(entity, def.type);

        // resources
        if (!def.modelId.empty()) {
            auto &assets = get_resource_manager(registry);
            if (const auto model_id = entt::hashed_string(def.modelId.c_str()); assets.models.contains(model_id)) {
                registry.emplace<WithModel>(entity, assets.models[model_id]);
            } else {
                TraceLog(LOG_WARNING, "Model %s not loaded for entity %s", def.modelId.c_str(), def.id.c_str());
            }
        }

        // location & orientation
        registry.emplace<Position3D>(entity, def.position);
        registry.emplace<Heading>(entity, def.heading);

        // friend of foe
        registry.emplace<FriendFoe>(entity, def.faction);

        switch (def.type) {
            case EntityType::CARRIER:
                registry.emplace<Landable>(entity, true);
                registry.emplace<Carrier>(entity);
                break;
            case EntityType::AIRBASE:
                registry.emplace<Landable>(entity, false);
                break;
            default:
                break;
        }

        TraceLog(LOG_DEBUG, "Entity %s created", def.id.c_str());
        return entity;
    }
}
