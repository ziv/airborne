module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module Prefabs:Unit;

import JsonConfig;
import Types;
import ResourceManager;
import Components;

export namespace factories {
entt::entity create_unit(entt::registry &registry, const EntityDef &def) {
  const auto entity = registry.create();

  registry.emplace<Identify>(entity, def.id, "");
  registry.emplace<IdentifyType>(entity, def.type);

  // location & orientation
  registry.emplace<Position3D>(entity, def.position);
  registry.emplace<Heading>(entity, def.heading);

  // friend of foe
  registry.emplace<FriendFoe>(entity, def.faction);

  // resources
  if (!def.model_id.empty()) {
    auto &assets = get_resource_manager(registry);
    if (const auto model_id = entt::hashed_string(def.model_id.c_str()); assets.models.contains(model_id)) {
      registry.emplace<WithModel>(entity, model_id);
    } else {
      TraceLog(LOG_WARNING, "Model %s not loaded for entity %s", def.model_id.c_str(), def.id.c_str());
    }
  }

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

  TraceLog(LOG_DEBUG, "entity %s created", def.id.c_str());
  return entity;
}
}  // namespace factories
