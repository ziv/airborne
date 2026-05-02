# Excessive Code Review

Scope: full repository pass over the C++ modules game code, resource loading, tile streaming, rendering systems, ECS usage, build files, and assets/config entry points. This review focuses on correctness, memory/resource lifetime, async/threading, performance, security, and early-project maintainability.

## Executive Summary

The project has a workable early ECS shape and the major game systems are easy to locate. The highest-risk area is the streaming/resource layer: remote tile failures, missing environment variables, unchecked raylib handles, and GPU resource ownership are not consistently represented as explicit success/failure states. The next biggest risk is frame-time cost from always-on debug/UI work and per-frame ECS scans/sorts that will become visible as entity and tile counts grow.

The first cleanup pass should harden resource loading and tile downloading before adding more gameplay systems. That will also make future performance work measurable instead of chasing undefined behavior from invalid textures/images.

## High Severity

### 1. Missing map provider tokens can crash the game

- `src/core/tiles/terrain-streamer.cppm:311`
- `src/core/tiles/map-streamer.cppm:57`

Both streamers construct `std::string(std::getenv("MAPBOX_TOKEN"))` / `std::string(std::getenv("TOMTOM_TOKEN"))` directly. If the env var is absent, `getenv` returns `nullptr`; constructing `std::string` from `nullptr` is undefined behavior and commonly crashes.

This can happen on a fresh checkout as soon as streaming asks for a tile. The local cache does not fully protect this path because the URL is built before `download()` checks whether the file already exists.

Recommendation: read the pointer first, validate it, and support one of these explicit states:

- remote streaming disabled, cached tiles only;
- startup error with a clear missing-token message;
- configured provider credentials from a config object.

### 2. Tile download failures are promoted to valid resources

- `src/core/tiles/tile-downloader.cppm:86-95`
- `src/core/tiles/tile-downloader.cppm:116-122`
- `src/core/tiles/map-streamer.cppm:148-160`
- `src/core/tiles/terrain-streamer.cppm:230-249`

`download()` logs and returns on HTTP, network, or file errors. The worker still resolves the promise as success, sometimes with `LoadImage(path)` after the file was not created. Consumers then call `LoadTextureFromImage()` and cache the result without checking `Image.data`, dimensions, or texture id.

Effects:

- invalid `Image` objects can be cached in `rm.images`;
- invalid GPU textures can be cached in `rm.textures`;
- height sampling can read from bad image metadata/data;
- rendering can operate on zero/invalid texture ids;
- failures are silent after a warning log, so retry/debug behavior is unclear.

Recommendation: make tile download return `expected<Image, TileError>` or set exceptions on promises. Only cache decoded images and textures after validation. Destroy or mark failed async tile entities with retry/backoff state rather than converting them to `TerrainChunk`/`MapTile`.

### 3. HTTPS certificate verification is disabled

- `src/core/tiles/tile-downloader.cppm:110-115`

`cli.enable_server_certificate_verification(false)` disables TLS verification for Mapbox/TomTom downloads. Terrain, map, and heightmap bytes are cached to disk and later decoded/rendered, so this is cache poisoning and local asset poisoning, not just a transient display risk.

Recommendation: enable verification by default. If a development bypass is needed, put it behind an explicit debug-only config or env var and log it once at startup.

### 4. Terrain streamer leaks a shader and generated models

- `src/core/tiles/terrain-streamer.cppm:89-99`
- `src/core/tiles/terrain-streamer.cppm:97-99`
- `src/core/tiles/terrain-utils.cppm:58`

`terrain_streamer::streamer` owns `Shader displacement_shader` and three heap-allocated `Model` objects created by `LoadShader()` / `LoadModelFromMesh()`. There is no destructor that calls `UnloadShader()` or `UnloadModel()`. The `unique_ptr<Model>` frees the C++ `Model` struct allocation only; it does not release the GPU buffers/material resources owned by raylib.

The current single-game flow may hide this, but it leaks GPU resources across screen/game recreation and at process shutdown.

Recommendation: use the existing `RaylibResource` wrappers or add a `~streamer()` that unloads `terrain_model12/13/14` and `displacement_shader`. Prefer value members with explicit RAII handles rather than `std::unique_ptr<Model>`.

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

### 7. Resource cache loaders do not validate raylib load success

- `src/core/resources/resource-manager.cppm:14-18`
- `src/core/resources/resource-manager.cppm:30-37`
- `src/core/resources/resource-manager.cppm:52-55`
- `src/core/resources/resource-preloader.cppm:18-30`

The loader wrappers always call `Unload*` in destructors, but they do not know whether loading succeeded. For missing files or decode failures, invalid raylib resources can still enter the cache. This interacts badly with the download failure path, but it also applies to scenario resources and shaders.

Recommendation: centralize validated loaders:

- texture/model/render texture: verify non-zero `id`;
- image: verify `data != nullptr`, `width > 0`, `height > 0`;
- shader: verify `id != 0`;
- music/sound: verify stream/buffer handles where raylib exposes them.

Failed loads should return explicit errors and should not be cached.

### 8. Terrain height images are intentionally kept in CPU memory, but eviction is all-or-nothing and unbounded between updates

- `src/core/tiles/terrain-streamer.cppm:64-76`
- `src/core/tiles/terrain-streamer.cppm:246-249`
- `src/core/tiles/terrain-utils.cppm:52-54`

The streamer stores both GPU height textures and CPU `Image` copies for ground-height sampling. That is reasonable, but it is expensive. A 1024x1024 RGB/RGBA heightmap is roughly 3-4 MB CPU memory per tile, plus GPU texture memory. During movement, rendered/pending tiles can coexist until coverage/range eviction runs, so memory can spike.

Recommendation: set a memory budget for height images. Consider keeping CPU height data in a compact format (`uint16_t`/`float` height grid) instead of full decoded RGBA images, and make eviction budget-driven rather than only desired/rendered set-driven.

### 9. Async tile jobs cannot be cancelled after entities are evicted

- `src/core/tiles/terrain-streamer.cppm:156-166`
- `src/core/tiles/terrain-streamer.cppm:319-323`
- `src/core/tiles/map-streamer.cppm:134-140`
- `src/core/tiles/map-streamer.cppm:173-176`

When a tile entity is no longer desired, the entity can be destroyed, but the queued/in-flight download continues. The global downloader has no cancellation and no bounded queue policy beyond four worker threads. Fast movement or zoom changes can enqueue work that is obsolete before it completes.

Recommendation: add cancellation/obsolescence checks. A simple early version: attach a generation id or desired-set lookup and discard decoded results for obsolete tiles before texture creation. Later, add queue dedupe by URL/path plus a bounded priority queue around camera distance.

### 10. Map streamer stacks `std::async` on top of the downloader thread pool

- `src/core/tiles/map-streamer.cppm:55-60`
- `src/core/tiles/map-streamer.cppm:173-176`
- `src/core/tiles/tile-downloader.cppm:181-190`

Map tiles use `std::async(std::launch::async)` per tile, and each async task blocks on the downloader pool with `enqueue(...).wait()`. That creates extra OS threads whose only job is to wait for another worker thread. This is unnecessary scheduling overhead and makes shutdown behavior less predictable.

Recommendation: expose an `enqueue_and_load()` path for map tiles too, or have the map streamer use the same downloader future directly. Keep one queueing abstraction.

### 11. `threads` debug counter is not an accurate thread count and can drift

- `src/core/tiles/terrain-streamer.cppm:89`
- `src/core/tiles/terrain-streamer.cppm:222-232`
- `src/core/tiles/terrain-streamer.cppm:319-321`

`threads += 2` counts futures, not actual threads. The downloader has a fixed four-thread pool. If a tile entity is destroyed before completion, or if a future errors after failure handling is improved, this counter can become wrong.

Recommendation: rename it to `pending_tile_jobs` and decrement on all completion/failure/discard paths, or query real queue/in-flight stats from the downloader.

### 12. Duplicate resource ids are not handled deliberately

- `src/screens/loading.cppm:31`
- `src/core/resources/resource-manager.cppm:99-110`

The loading screen blindly calls `rm.*.load(res_id, path)`. Duplicate ids, mission reloads, or screen restarts are not checked. Depending on EnTT cache semantics, this can fail to replace resources, return existing stale resources, or make duplicate config hard to detect.

Recommendation: make loading idempotent and explicit. Either reject duplicate ids as scenario errors or validate that an existing id has the same type/path and skip it.

### 13. Game destruction clears entities but leaves context/global state behind

- `src/game.cppm:77-80`
- `src/utils/accessors.cppm:12-16`
- `src/core/resources/resource-manager.cppm:72-85`

`Game::~Game()` calls `registry.clear()`, but global context values (`PlayerEntity`, `GameState`, `Configuration`, `GameOptions`, `ResourceManager`) remain. A later `Game` instance calls `create_player()` and `registry.ctx().emplace<PlayerEntity>(player)`, which will fail if the context already contains `PlayerEntity`.

Recommendation: separate per-game state from app-global state. On game teardown, erase per-game context values or use `insert_or_assign`/`emplace_or_replace` where reload is intended. Do not rely on process exit as the only valid lifecycle.

### 14. Landing-zone checks are O(number of landables) every frame

- `src/systems/player/player-position.cppm:23-63`
- `src/systems/player/player-position.cppm:98-113`

Every frame, player position checks every `Landable` entity and performs trig for heading rotation. This is fine for a few carriers/airbases, but it will scale poorly if scenario entities grow or if more landable/trigger volumes are added.

Recommendation: precompute landing zone transform/half extents on a component, and only query nearby zones. A simple spatial hash or broad-phase grid is enough.

### 15. Ground-height sampling checks only a fixed zoom chain and assumes loaded images are valid

- `src/core/tiles/terrain-streamer.cppm:57-76`
- `src/core/tiles/terrain-utils.cppm:96-120`

`ground_height_at()` samples z14, z13, z12 only and returns `0.0f` if no image is loaded. That means terrain collision can temporarily flatten to sea level while tiles load or fail. The helper clamps pixel coordinates but does not verify `img.data` before casting it.

Recommendation: track terrain availability separately from terrain height. If no valid height tile is available, keep the last known safe height, use a coarse fallback, or mark physics as waiting. Validate image data before sampling.

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

