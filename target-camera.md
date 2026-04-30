# Target Camera Widget

## Context

The simulator has three dashboard slots cycling through Minimap → Radar → Engine → TargetCamera via F2/F3/F4. `TargetCameraWidget` tag and `updates::set_target_camera()` already exist. The renderer stub at `render-target-camera.cppm` is empty. This plan implements it.

The widget shows a 3D sub-view of the currently locked target (`RadarState.locked_target`) rendered into the slot using a secondary `Camera3D` aimed at the target, drawn into a `RenderTexture2D` owned by the `ResourceManager`.

Resources are never stored directly on components — only their integer ID (hashed string key into the relevant `ResourceManager` cache).

---

## 1. Add `RenderTextureResourceLoader` + cache to `ResourceManager`

`src/core/resources/resource-manager.cppm`:

```cpp
export struct RenderTextureResourceLoader {
  RenderTexture2D res;
  explicit RenderTextureResourceLoader(RenderTexture2D rt) : res(rt) {}
  ~RenderTextureResourceLoader() { UnloadRenderTexture(res); }
  RenderTextureResourceLoader(const RenderTextureResourceLoader&) = delete;
  RenderTextureResourceLoader& operator=(const RenderTextureResourceLoader&) = delete;
};

export struct ResourceManager {
  entt::resource_cache<TextureResourceLoader>       textures;
  entt::resource_cache<ModelResourceLoader>         models;
  entt::resource_cache<ImageResourceLoader>         images;
  entt::resource_cache<ShaderLoader>                shaders;
  entt::resource_cache<MusicStreamResourceLoader>   music_streams;
  entt::resource_cache<SoundResourceLoader>         sounds;
  entt::resource_cache<RenderTextureResourceLoader> render_textures;  // NEW
};
```

---

## 2. Component — `src/components/components.cppm`

```cpp
export struct TargetCameraWidget {
  int render_tex_id = 0;  // key into rm.render_textures
  int size = 150;
};
```

---

## 3. `set_target_camera` — `src/prefabs/create-cockpit-widgets.cppm`

Generate a stable ID from the slot index, create the render texture, load into cache:

```cpp
void set_target_camera(const int slot, entt::registry &registry) {
  for (const auto [entity, dashboard] : registry.view<DashboardSlot>().each()) {
    if (dashboard.slot_index != slot) continue;

    auto &rm = get_resource_manager(registry);
    constexpr int size = 150;
    const int rt_id = entt::hashed_string(TextFormat("target_cam_rt_%d", slot)).value();

    // Erase old texture if already present (cycling back into this widget)
    rm.render_textures.erase(rt_id);
    rm.render_textures.load(rt_id, LoadRenderTexture(size, size));

    TargetCameraWidget w;
    w.size          = size;
    w.render_tex_id = rt_id;
    registry.emplace_or_replace<TargetCameraWidget>(entity, w);
    break;
  }
}
```

---

## 4. Widget removal — `src/systems/aircraft/widgets-inputs.cppm`

Erase from cache (RAII loader unloads the GPU texture automatically):

```cpp
if (registry.all_of<TargetCameraWidget>(entity)) {
  const auto &w = registry.get<TargetCameraWidget>(entity);
  get_resource_manager(registry).render_textures.erase(w.render_tex_id);
  registry.remove<TargetCameraWidget>(entity);
  updates::set_minimap(slotIndex, registry);
  continue;
}
```

---

## 5. Renderer — `src/systems/renderers/render-target-camera.cppm`

```cpp
export namespace render_systems {
void target_camera(entt::registry &registry) {
  const auto view     = registry.view<DashboardSlot, TargetCameraWidget, Position2D>();
  const auto &player  = get_player(registry);
  const auto &radar   = registry.get<RadarState>(get_player_entity(registry));
  const auto &rm      = get_resource_manager(registry);

  for (auto [entity, slot, wd, pos] : view.each()) {
    const int wx = static_cast<int>(pos.pos.x);
    const int wy = static_cast<int>(pos.pos.y);
    const int sz = wd.size;

    // No render texture ready — skip
    if (!rm.render_textures.contains(wd.render_tex_id)) continue;
    const RenderTexture2D &rt = rm.render_textures[wd.render_tex_id]->res;

    // No lock — placeholder
    if (radar.locked_target == entt::null || !registry.valid(radar.locked_target)) {
      DrawRectangle(wx, wy, sz, sz, BLACK);
      DrawText("NO TARGET", wx + 10, wy + sz / 2 - 8, 12, DARKGREEN);
      continue;
    }

    const auto  &target_pos  = registry.get<Position3D>(radar.locked_target).pos;
    const Vector3 player_abs = player.absolute_position();
    const float   distance   = Vector3Distance(player_abs, target_pos);
    const int     alt_feet   = static_cast<int>(meter_to_feet(target_pos.y));

    // Secondary camera: player position → target
    Camera3D cam{};
    cam.position   = player_abs + player.offset;
    cam.target     = target_pos + player.offset;
    cam.up         = {0.0f, 1.0f, 0.0f};
    cam.fovy       = 30.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    // Draw only the locked target's model (cheap — no full scene redraw)
    BeginTextureMode(rt);
      ClearBackground({10, 10, 20, 255});
      BeginMode3D(cam);
        if (const auto *modeled = registry.try_get<WithModel>(radar.locked_target)) {
          if (rm.models.contains(modeled->model)) {
            const float hdg = registry.all_of<Heading>(radar.locked_target)
                            ? registry.get<Heading>(radar.locked_target).heading : 0.0f;
            DrawModelEx(rm.models[modeled->model]->res,
                        target_pos + player.offset,
                        {0.0f, 1.0f, 0.0f}, hdg, {1.0f, 1.0f, 1.0f}, WHITE);
          }
        }
      EndMode3D();
    EndTextureMode();

    // RenderTexture is flipped vertically in raylib
    const Rectangle src  = {0, 0, static_cast<float>(sz), -static_cast<float>(sz)};
    const Rectangle dest = {static_cast<float>(wx), static_cast<float>(wy),
                            static_cast<float>(sz),  static_cast<float>(sz)};
    DrawTexturePro(rt.texture, src, dest, {0, 0}, 0.0f, WHITE);

    // Overlay
    DrawRectangleLines(wx, wy, sz, sz, DARKGREEN);
    if (registry.all_of<Identify>(radar.locked_target))
      DrawText(registry.get<Identify>(radar.locked_target).name.c_str(), wx + 4, wy + 4, 10, GREEN);
    DrawText(TextFormat("%.1f km", distance / 1000.0f), wx + 4, wy + sz - 28, 10, GREEN);
    DrawText(TextFormat("%d ft",   alt_feet),            wx + 4, wy + sz - 16, 10, GREEN);
  }
}
}
```

---

## 6. `src/systems/renderers/render.cppm`

```cpp
export import :TargetCamera;
```

## 7. `src/game.cppm` — `draw()`

```cpp
render_systems::target_camera(registry);  // after RenderRadar
```

---

## Files summary

| File | Change |
|------|--------|
| `src/core/resources/resource-manager.cppm` | Add `RenderTextureResourceLoader` + `render_textures` cache |
| `src/components/components.cppm` | `TargetCameraWidget` gets `int render_tex_id` + `int size` |
| `src/prefabs/create-cockpit-widgets.cppm` | `set_target_camera` loads render texture into cache by ID |
| `src/systems/aircraft/widgets-inputs.cppm` | Erase from cache before `remove<TargetCameraWidget>` |
| `src/systems/renderers/render-target-camera.cppm` | Full implementation |
| `src/systems/renderers/render.cppm` | `export import :TargetCamera` |
| `src/game.cppm` | Call `render_systems::target_camera(registry)` in `draw()` |

---

## Verification

1. Cycle a slot to TargetCamera — shows "NO TARGET" on black
2. Press T to lock an aircraft — sub-view shows model with name, distance, altitude overlay
3. Target leaves range → locked_target goes null → "NO TARGET"
4. Cycle away — cache erases render texture, GPU memory freed
