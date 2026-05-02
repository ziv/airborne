module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module AircraftSystems:WidgetsInputs;

import Components;
import Prefabs;
import ResourceManager;
import Accessors;

void change_radar_range(entt::registry &registry) {
  for (const auto radar_view = registry.view<RadarWidget>(); const auto [entity, radar] : radar_view.each()) {
    radar.rangeIndex = (radar.rangeIndex + 1) % static_cast<int>(radar.cfg.ranges.size());
  }
}

void change_hud_colors(entt::registry &registry) {
  for (const auto hud_view = registry.view<HudWidget>(); const auto [entity, hud] : hud_view.each()) {
    hud.colorIndex = (hud.colorIndex + 1) % 5;
  }
}

void change_slot_widget(entt::registry &registry, const int slotIndex) {
  for (const auto slots_view = registry.view<DashboardSlot>(); const auto [entity, slot] : slots_view.each()) {
    if (slot.slot_index != slotIndex) continue;
    // available widgets to put in slots: minimap, radar, engine

    if (registry.all_of<MinimapWidget>(entity)) {
      registry.remove<MinimapWidget>(entity);  // todo what about cleaning the texture?
      // set radar
      updates::set_radar(slotIndex, registry);
      continue;
    }
    if (registry.all_of<RadarWidget>(entity)) {
      registry.remove<RadarWidget>(entity);
      // set engine status
      updates::set_engine_status(slotIndex, registry);
      continue;
    }
    if (registry.all_of<EngineWidget>(entity)) {
      registry.remove<EngineWidget>(entity);
      // set target camera
      updates::set_target_camera(slotIndex, registry);
      continue;
    }
    if (registry.all_of<TargetCameraWidget>(entity)) {
      const auto &w = registry.get<TargetCameraWidget>(entity);
      get_resource_manager(registry).render_textures.erase(w.render_tex_id);
      registry.remove<TargetCameraWidget>(entity);
      updates::set_minimap(slotIndex, registry);
      continue;
    }
  }
}

export namespace aircraft_systems {
void widgets_inputs(entt::registry &registry) {
  if (IsKeyPressed(KEY_R)) change_radar_range(registry);

  if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_H)) change_hud_colors(registry);

  if (IsKeyPressed(KEY_F2)) change_slot_widget(registry, 2);
  if (IsKeyPressed(KEY_F3)) change_slot_widget(registry, 0);
  if (IsKeyPressed(KEY_F4)) change_slot_widget(registry, 1);
}
}  // namespace aircraft_systems