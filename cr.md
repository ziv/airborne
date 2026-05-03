# Excessive Code Review

### 5. Render textures can leak when widgets are replaced or the game is destroyed

- `src/prefabs/create-cockpit-widgets.cppm:85-88`
- `src/systems/aircraft/widgets-inputs.cppm:49-53`
- `src/game.cppm:77-80`
- `src/core/resources/resource-manager.cppm:76-85`

Target-camera render textures are stored in the global `ResourceManager`, while `Game::~Game()` only calls `registry.clear()`. Clearing entities removes `TargetCameraWidget`, but it does not erase render textures from the resource manager. `unload_resource_manager()` eventually clears them at process shutdown, but a game/screen reload would accumulate GPU render textures unless every widget transition happens to erase them manually first.

Recommendation: define ownership. If a render texture is tied to a widget entity, unload it in a widget cleanup/system or a component lifecycle handler. If it is a global resource, create it once and reuse it. Avoid hidden GPU resources whose lifetime outlives their ECS owners.

### 6. Locked target access assumes components that may not exist

- `src/systems/renderers/render-target-camera.cppm:29-36`
- `src/systems/renderers/render-debug.cppm:76-84`

`render-target-camera` verifies `registry.valid(radar.locked_target)`, then directly reads `Position3D` and `IdentifyType`. `RenderDebug` reads `Position3D` when `locked_target != entt::null` without even checking `registry.valid()`.

Today `update_lock()` only selects entities from a view with those components, but ECS state can change independently: destroyed targets, component removal, mode changes, or future systems can invalidate that assumption. In debug rendering, a stale `locked_target` can crash the frame.

Recommendation: use `try_get<Position3D>()` / `try_get<IdentifyType>()` at render sites and clear the lock when required components are missing. Treat `entt::entity` handles as weak references.

## Medium Severity

### 9. Async tile jobs cannot be cancelled after entities are evicted

- `src/core/tiles/terrain-streamer.cppm:156-166`
- `src/core/tiles/terrain-streamer.cppm:319-323`
- `src/core/tiles/map-streamer.cppm:134-140`
- `src/core/tiles/map-streamer.cppm:173-176`

When a tile entity is no longer desired, the entity can be destroyed, but the queued/in-flight download continues. The global downloader has no cancellation and no bounded queue policy beyond four worker threads. Fast movement or zoom changes can enqueue work that is obsolete before it completes.

Recommendation: add cancellation/obsolescence checks. A simple early version: attach a generation id or desired-set lookup and discard decoded results for obsolete tiles before texture creation. Later, add queue dedupe by URL/path plus a bounded priority queue around camera distance.

### 16. Per-frame render path copies ECS components unnecessarily

- `src/systems/renderers/render-hud.cppm:207-211`
- `src/systems/aircraft/update-lock-system.cppm:14`
- `src/systems/player/player-position.cppm:69`
- `src/systems/player/player-physics.cppm:15`

Several systems copy component/config structs that are read-only within the frame. `Player` is not huge, but these copies add up as systems grow and make mutation intent less clear.

Recommendation: prefer `const auto&` for read-only components/config and `auto&` only when mutating. This also matches the TODO in the README about read-only view data.

### 17. Target lock sorting recomputes square roots in the comparator

- `src/systems/aircraft/update-lock-system.cppm:64-68`
- `src/systems/aircraft/update-lock-system.cppm:81-86`

On `T`, valid targets are collected with `Vector3Distance()` and then sorted using a comparator that calls `Vector3Distance()` repeatedly. Sorting calls the comparator O(n log n), so this does repeated square roots and registry lookups.

Recommendation: store `{entity, distanceSq}` once, sort by `distanceSq`, and use squared range checks.

### 18. Minimap tile size uses `pow()` every frame

- `src/systems/renderers/render-minimap.cppm:14`
- `src/systems/renderers/render-minimap.cppm:21-23`

`minimap_tile_size()` uses `std::pow(2.0f, 12 - zoom)` in the render loop. Zoom is an integer range and tile size can be computed with shifts or cached per zoom.

Recommendation: precompute sizes for zoom 1-20 or use integer shifts like `map_tile_size()` already does in `src/core/tiles/map-streamer.cppm`.

### 19. HUD ladder redraws a large fixed pitch ladder every frame

- `src/systems/renderers/render-hud.cppm:21-59`

The ladder draws all marks from -180 to 180 every frame, then relies on scissor clipping. This is not a crisis, but it does extra CPU draw calls and `TextFormat()` calls for marks that are far outside the visible ladder.

Recommendation: compute visible pitch range from current pitch/scissor height and only draw marks that can appear. Consider pre-rendering static ladder text/geometry if HUD cost becomes measurable.

### 20. Always-on debug overlays create steady frame cost and visual noise

- `src/game.cppm:146-151`
- `src/core/tiles/terrain-streamer.cppm:149-151`
- `src/systems/renderers/render-debug.cppm:13-85`

`RenderDebug()`, `streamer.show_debug_data()`, and `DrawFPS()` run unconditionally in `Game::draw()`. `RenderDebug()` performs many `TextFormat()` calls and may crash on stale target locks as noted above.

Recommendation: gate debug overlays behind `GameOptions`, a compile flag, or a debug key. Keep debug rendering out of normal performance measurements.

### 21. The terrain LOD bookkeeping allocates vectors in helper functions

- `src/core/tiles/terrain-utils.cppm:71-88`
- `src/core/tiles/terrain-streamer.cppm:346-363`

`children()` and `grand_children()` allocate `std::vector<TileKey>` when `is_tile_covered()` runs. This is small, but it is on a path that can execute for many tiles during LOD transitions.

Recommendation: return `std::array<TileKey, 4>` / `std::array<TileKey, 16>` or inline loops with no heap allocation.

### 22. Terrain desired/rendered sets use `std::map` for small hot sets

- `src/core/tiles/terrain-streamer.cppm:84-85`
- `src/core/tiles/terrain-streamer.cppm:156-218`
- `src/core/tiles/map-streamer.cppm:105`

The tile sets are small today, but they are hot and updated frequently. `std::map` allocates one node per tile and has poor cache locality. The map streamer also uses `std::set` for a 3x3 desired grid.

Recommendation: use sorted `std::vector`, `std::unordered_map`, or a flat hash map once tile counts grow. For the minimap desired set, a fixed array/vector is simpler and cheaper than `std::set`.

### 23. Model rendering has culling disabled/commented out

- `src/systems/renderers/render-models.cppm:14`
- `src/systems/renderers/render-models.cppm:28-33`

The render distance and front-of-camera checks are present but commented. All modeled entities are submitted every frame. This will be fine with the current scenario, but it will not scale with a larger battlefield.

Recommendation: re-enable distance culling using squared distance. Add frustum or coarse sector culling later if entity counts grow.

### 24. Target camera renders an extra 3D pass each frame when locked

- `src/systems/renderers/render-target-camera.cppm:65-79`

The target camera uses `BeginTextureMode()` and a 3D draw every frame for the locked target. At 150x150 this is cheap, but the pattern matters if more camera widgets or higher sizes are added.

Recommendation: update the render texture at a lower frequency, only when the target/relative camera changes materially, or budget it as an optional cockpit feature.

### 25. `TextFormat()` is heavily used in per-frame paths

- `src/systems/renderers/render-hud.cppm:53-54`
- `src/systems/renderers/render-hud.cppm:104-136`
- `src/systems/renderers/render-debug.cppm:38-84`
- `src/systems/renderers/render-radar.cppm:25-26`
- `src/core/tiles/terrain-streamer.cppm:149-151`

Raylib `TextFormat()` uses a static internal buffer. It is convenient, but repeated calls across a frame have overhead and can be error-prone if pointers are retained. The current code mostly uses it immediately, which is safe, but the total call volume is high in debug/HUD paths.

Recommendation: gate debug paths, precompute static label strings, and avoid formatting offscreen HUD marks.

## Low Severity / Maintainability

### 26. README build requirements conflict with CMake

- `CMakeLists.txt:1`
- `readme.md:94`

README says CMake 3.25 or higher, while `CMakeLists.txt` requires 4.2.2.

Recommendation: document the real supported CMake/LLVM/Ninja versions or lower the minimum if 4.2.2 is not necessary.

### 27. `ccache` is forced for every build

- `CMakeLists.txt:2`

`set(CMAKE_CXX_COMPILER_LAUNCHER ccache)` assumes `ccache` exists and is writable. In this review, the sandboxed build failed because ccache tried to write outside the workspace.

Recommendation: enable ccache conditionally:

```cmake
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()
```

### 28. `save_json_to_file()` silently ignores write failures

- `src/core/json-config.cppm:25-31`

Failed writes are silently dropped. This will make options/config persistence hard to diagnose.

Recommendation: return `bool`/`expected` or throw/log a clear error.

### 29. Static shader uniform location can go stale

- `src/systems/renderers/render-models.cppm:47-49`

`time_loc` is cached in a function-local `static` based on the first sky shader. If the shader is recreated/reloaded, the cached location may refer to the old program.

Recommendation: store uniform locations with the shader resource or recompute after shader creation.

### 30. Scenario parsing has two competing paths

- `src/core/types/scenario-types.cppm:150-197`
- `src/prefabs/create-unit.cppm:15-47`
- `src/prefabs/spawn-entities.cppm:13-141`

There is a typed `EntityDef` path and a dynamic JSON component-spawning path. The current scenario uses the dynamic path. The typed path appears partially stale (`modelId` vs dynamic `"model"`, different component layout).

Recommendation: pick one source of truth for scenario schema. For early-stage iteration, dynamic JSON is fine, but add validation so unknown/missing fields fail fast.

### 31. `GameScreen` contains dead/commented loading code

- `src/screens/game.cppm:18-35`
- `src/screens/game.cppm:66-88`

There is an unused `make_loading_sequence()` and a commented resource-loading state machine. The active loading screen already handles resources.

Recommendation: remove the dead path or move the generator to the loading screen. Early cleanup here will make lifecycle bugs easier to see.

## Memory / Resource Ownership Notes

The project already has two RAII strategies:

- `RaylibResource` in `src/core/resources/raylib-resource.cppm`
- EnTT resource-cache wrappers in `src/core/resources/resource-manager.cppm`

The main issue is inconsistency. Splash screen resources use `RaylibResource`, cached resources use `ResourceLoader`, terrain streamer uses raw raylib handles, and target render textures are cache-owned but widget-created. Pick one ownership model per resource category:

- app-global assets: `ResourceManager`;
- entity-owned temporary GPU resources: component with RAII handle or component lifecycle cleanup;
- streamer-owned resources: RAII members in the streamer.

Avoid raw raylib handles as owning members unless the same class has a destructor that unloads them.

## Performance Hotspots To Measure First

1. Terrain streaming update and `process_loaded_chunks()`.
2. CPU/GPU memory used by terrain textures + height images.
3. Render submit count with terrain chunks and models visible.
4. HUD/debug `DrawText`/`TextFormat` overhead.
5. Lock/radar scans as scenario entity count grows.

Suggested instrumentation:

- add frame-time buckets around `Game::update()` systems and `Game::draw()` sections;
- expose tile counts, queued jobs, loaded CPU images, GPU textures, and failed downloads;
- add a debug toggle so profiling can compare normal vs debug-overlay cost.

## Prioritized Remediation Plan

1. Harden env-var handling and remote streaming startup behavior.
2. Make tile downloader return explicit success/failure; stop caching invalid images/textures.
3. Restore HTTPS certificate verification.
4. Put terrain streamer shader/models under RAII.
5. Define render texture ownership and cleanup on widget/game teardown.
6. Add resource validation wrappers before inserting into EnTT caches.
7. Gate debug overlays and remove unconditional debug render work from normal gameplay.
8. Replace obvious per-frame copies and repeated distance sqrt/sort work.
9. Add tile memory/queue budgets before increasing terrain LOD or map coverage.
10. Align README/CMake build requirements and conditionalize ccache.

## Verification

- `cmake --build cmake-build-debug` initially failed in the sandbox because `ccache` writes to `/Users/ziv.perry/Library/Caches/ccache`.
- Re-running the same build with permission succeeded and linked `cmake-build-debug/airborne`.
- No runtime smoke test was performed because launching a raylib window is outside this headless review pass.

