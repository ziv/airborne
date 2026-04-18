module;
#include <entt/entt.hpp>
#include "../lib/ray.hpp"

export module WidgetsInputs;

import Components;

void increase_minimap_zoom(entt::registry &registry) {
    for (const auto minimap_view = registry.view<MinimapWidget>(); const auto [entity, minimap]: minimap_view.each()) {
        minimap.zoom += 0.4f;
    }
}

void decrease_minimap_zoom(entt::registry &registry) {
    for (const auto minimap_view = registry.view<MinimapWidget>(); const auto [entity, minimap]: minimap_view.each()) {
        minimap.zoom -= 0.4f;
    }
}

void change_radar_range(entt::registry &registry) {
    for (const auto radar_view = registry.view<RadarWidget>(); const auto [entity, radar]: radar_view.each()) {
        radar.rangeIndex = (radar.rangeIndex + 1) % radar.cfg.ranges.size();
    }
}

void change_hud_colors(entt::registry &registry) {
    for (const auto hud_view = registry.view<HudWidget>(); const auto [entity, hud]: hud_view.each()) {
        hud.colorIndex = (hud.colorIndex + 1) % 5;
    }
}

void change_slot_widget(entt::registry &registry, const int slotIndex) {
    for (const auto slots_view = registry.view<DashboardSlot>(); const auto [entity, slot]: slots_view.each()) {
        if (slot.slot_index != slotIndex) continue;
        // available widgets to put in slots: minimap, radar, engine

        if (registry.all_of<MinimapWidget>(entity)) {
            registry.remove<MinimapWidget>(entity);
            // set radar
            continue;;
        }
        if (registry.all_of<RadarWidget>(entity)) {
            registry.remove<RadarWidget>(entity);
            // set engine status
            continue;;
        }
        if (registry.all_of<EngineWidget>(entity)) {
            registry.remove<EngineWidget>(entity);
            // set minimap
        }
    }
}

export void WidgetsInputs(entt::registry &registry) {
    if (IsKeyPressed(KEY_Z)) increase_minimap_zoom(registry);
    if (IsKeyPressed(KEY_X)) decrease_minimap_zoom(registry);

    if (IsKeyPressed(KEY_R)) change_radar_range(registry);

    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_H)) change_hud_colors(registry);

    if (IsKeyPressed(KEY_F2)) change_slot_widget(registry, 2);
    if (IsKeyPressed(KEY_F3)) change_slot_widget(registry, 0);
    if (IsKeyPressed(KEY_F4)) change_slot_widget(registry, 1);
}
