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
import Accessors;

export namespace updates {
template <typename WidgetType>
void remove_widget(const int slot, entt::registry &registry) {
  const auto view = registry.view<DashboardSlot>();
  for (const auto [entity, dashboard] : view.each()) {
    if (dashboard.slot_index != slot) continue;
    if (registry.all_of<WidgetType>(entity)) registry.remove<WidgetType>(entity);
  }
}

void set_engine_status(const int slot, entt::registry &registry) {
  const auto view = registry.view<DashboardSlot>();
  for (const auto [entity, dashboard] : view.each()) {
    if (dashboard.slot_index != slot) continue;
    registry.get_or_emplace<EngineWidget>(entity);
    break;
  }
}

void set_radar(const int slot, entt::registry &registry) {
  const auto view = registry.view<DashboardSlot>();
  for (const auto [entity, dashboard] : view.each()) {
    if (dashboard.slot_index != slot) continue;

    const auto radar_cfg = get_config(registry).views.radar;
    const auto size = static_cast<int>(radar_cfg.ranges.size());
    registry.emplace_or_replace<RadarWidget>(entity, radar_cfg, size, 0);
    break;
  }
}

void set_minimap(const int slot, entt::registry &registry) {
  for (const auto view = registry.view<DashboardSlot>(); const auto [entity, dashboard] : view.each()) {
    if (dashboard.slot_index != slot) continue;

    // auto &manager = get_resource_manager(registry);

    const MinimapConfig minimap_cfg = get_config(registry).views.minimap;
    MinimapWidget widget;
    widget.cfg      = minimap_cfg;
    widget.map_zoom = minimap_cfg.defaultMapZoom;
    registry.emplace_or_replace<MinimapWidget>(entity, widget);
    break;
  }
}

void set_minicam(const int slot, entt::registry &registry) {
  const auto view = registry.view<DashboardSlot>();
  for (const auto [entity, dashboard] : view.each()) {
    if (dashboard.slot_index != slot) continue;
    registry.get_or_emplace<CameraWidget>(entity);
    break;
  }
}

void set_target_camera(const int slot, entt::registry &registry) {
  for (const auto [entity, dashboard] : registry.view<DashboardSlot>().each()) {
    if (dashboard.slot_index != slot) continue;

    auto &rm = get_resource_manager(registry);
    constexpr int size = 150;
    const int rt_id = entt::hashed_string(TextFormat("target_cam_rt_%d", slot)).value();

    rm.render_textures.erase(rt_id);
    rm.render_textures.load(rt_id, LoadRenderTexture(size, size));

    registry.emplace_or_replace<TargetCameraWidget>(entity, rt_id, size);
    break;
  }
}
}  // namespace updates

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
}  // namespace factories
