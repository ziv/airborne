module;
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include "../lib/ray.hpp"

export module Prefabs:CockpitWidgets;

import JsonConfig;
import Components;
import RaylibResource;
import ResourceManager;
import Types;

export namespace updates {
    template<typename WidgetType>
    void remove_widget(const int slot, entt::registry &registry) {
        const auto view = registry.view<DashboardSlot>();
        for (const auto [entity, dashboard]: view.each()) {
            if (dashboard.slot_index != slot) continue;
            if (registry.all_of<WidgetType>(entity)) registry.remove<WidgetType>(entity);
        }
    }

    void set_engine_status(const int slot,
                         entt::registry &registry) {
        const auto view = registry.view<DashboardSlot>();
        for (const auto [entity, dashboard]: view.each()) {
            if (dashboard.slot_index != slot) continue;
            registry.get_or_emplace<EngineWidget>(entity);
            break;
        }
    }


    void set_radar(const int slot,
                  entt::registry &registry,
                  const JsonConfig &config) {
        const auto view = registry.view<DashboardSlot>();
        for (const auto [entity, dashboard]: view.each()) {
            if (dashboard.slot_index != slot) continue;

            const auto radar_cfg = config.get<RadarConfig>("/views/radar");
            constexpr auto size = static_cast<int>(radar_cfg.ranges.size());
            registry.emplace_or_replace<RadarWidget>(entity, radar_cfg, size, 0);
            break;
        }
    }

    void set_minimap(const int slot,
                    entt::registry &registry,
                    const JsonConfig &config) {
        for (const auto view = registry.view<DashboardSlot>(); const auto [entity, dashboard]: view.each()) {
            if (dashboard.slot_index != slot) continue;

            auto &manager = get_resource_manager(registry);

            const auto minimap_cfg = config.get<MinimapConfig>("/views/minimap");
            registry.emplace_or_replace<MinimapWidget>(entity, minimap_cfg);

            if (const auto tex_id = entt::hashed_string(minimap_cfg.mapTexture.c_str()); manager.textures.contains(tex_id)) {
                registry.emplace_or_replace<WithTexture>(entity, manager.textures[tex_id]);
            } else {
                TraceLog(LOG_WARNING, "minimap texture '%s' not found in cache", minimap_cfg.mapTexture.c_str());
            }
            break;
        }
    }
}

export namespace factories {
    void create_cockpit_widgets(entt::registry &registry) {
        // widget A
        const auto screen_a = registry.create();
        registry.emplace<DashboardSlot>(screen_a, 0);
        registry.emplace<Position2D>(screen_a, (Vector2){528.0f, 612.0});

        // widget B
        const auto screen_b = registry.create();
        registry.emplace<DashboardSlot>(screen_b, 1);
        registry.emplace<Position2D>(screen_b, (Vector2){752.0f, 597.0});

        // widget C
        const auto screen_c = registry.create();
        registry.emplace<DashboardSlot>(screen_c, 2);
        registry.emplace<Position2D>(screen_c, (Vector2){300.0f, 597.0});
    }
}
