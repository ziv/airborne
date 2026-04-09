# Copilot Instructions — Airborne

A modern remake of F-15 Strike Eagle II built from scratch in C++17 with raylib (no game engine). The goal is to recreate the original's gameplay feel — not a full simulator, not an arcade — something in between.

## Build

```sh
cmake --build cmake-build-debug
./cmake-build-debug/se
```

The project uses CMake with `FetchContent` to pull raylib 5.5. There are no tests or linters configured.

## Architecture

### Screen State Machine

`main.cpp` runs a screen state machine. Each screen inherits from `GameScreen` (virtual `update()` → returns next `ScreenState`, virtual `run()` → draws). Screens are swapped via `std::make_unique` on state transitions: `SPLASH → MAIN_MENU → GAMEPLAY`.

### Aircraft Simulation (src/core/)

`GameData` is the orchestrator. It composes and coordinates these components in order each frame:

1. **`AircraftControls`** — maps keyboard input to `PilotControls` (pitch, yaw, roll, throttle, brakes, gear)
2. **`AircraftPhysics`** — force-based flight dynamics (thrust, drag, lift, gravity). Handles flying vs ground modes with different drag models, fuel consumption, stall behavior, and ground clamping
3. **`AircraftTransformation`** — quaternion-based orientation updates. Applies adverse yaw, lift-loss pitch, and VLE turbulence effects
4. **`AircraftCamera`** — first-person pilot view derived from aircraft orientation

All components follow the `update(AircraftState&, float dt)` pattern for frame-rate independence.

### Views (src/views/)

Views are 2D overlay renderers drawn on top of the 3D scene. Each has `update()` and `draw()` methods. `GameplayScreen` composes all views and orchestrates the 2D/3D rendering passes. Key views: `CockpitView` (chroma-keyed cockpit texture), `MinihudView` (artificial horizon), `MapView` (top-down tactical map with Camera2D), `NavballView` (3D orientation ball), `RadarView`, `DebugView`.

### Terrain & Scene (src/services/, src/core/SceneManager)

`TileManager` handles async terrain chunk streaming (16×16 grid) using `std::future` with tile states: `LOADING_IMAGES → BUILDING_MESH → READY`. `SceneManager` manages static scene elements (terrain, carrier model, fog shader, clouds, engine sound).

### Large-World Precision

`AircraftState` uses a `mapOffset` (`Vector2`) alongside `position` (`Vector3`) to shift coordinates when they exceed `SHIFT_THRESHOLD` (5000.0f), preventing floating-point precision loss over large terrain.

## Configuration

All tunable parameters live in `res/config/app.jsonc` (JSON with comments). Access via `AppConfig::get<T>("/json/pointer/path")` using JSON pointer syntax. Classes receive `const AppConfig&` via constructor injection and extract values at init time.

Key config sections: `/airplane` (flight physics), `/pilot` (camera FOV/tilt), `/game` (terrain and asset paths), `/views` (HUD layout), `/autoPilot`.

## Conventions

- **Headers**: `#pragma once` throughout
- **Naming**: PascalCase classes, camelCase methods/members, SCREAMING_SNAKE for constants in namespaces
- **Memory**: RAII via `RaylibResource<T>` template (`TextureHandle`, `ModelHandle`, `ShaderHandle`, `MusicHandle` in `Resource.h`) for GPU resource cleanup. `std::unique_ptr` for screen ownership. No dynamic allocation in hot physics loops.
- **Physics types**: Type aliases in `Types.h` (`Newton`, `Meter`, `MeterPerSecond`, `Degree`, `Ratio`) with user-defined literals (`100.0_mps`, `5000.0_mtr`)
- **World constants**: `GamePhysics` namespace in `Constants.h` — world axes, gravity, unit conversion factors
- **Composition over inheritance**: `GameData` composes controllers; `GameplayScreen` composes views
- **Const-correctness**: `const` on read methods, `[[nodiscard]]` on important queries

## Resources

Assets are in `res/` — terrain textures/heightmaps, 3D models (.glb), cockpit sprites, shaders (GLSL in `res/shaders/`), and audio. The `split_map.py` utility splits large map images into tile grids. The `devblog/` directory is a Hugo site (separate from the game code).
