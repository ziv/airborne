# Copilot Instructions — Airborne

## Rules

1. **Never make changes I didn't ask for.** Always verify with me before making any unrequested modifications.
2. **Assume any unexpected code changes were made by me.** If something changed and you don't know why, ask me — don't try to fix or revert it.

## Project Overview

A modern remake of F-15 Strike Eagle II — not a full simulator, not an arcade, but something in between. Built from scratch with raylib (no game engine). The codebase uses C++23 modules and an ECS architecture (EnTT).

## Build

Requires LLVM/Clang with C++20 module support and CMake 4.2+. Uses Ninja generator and ccache.

```shell
# Configure (one-time, macOS with Homebrew LLVM)
export CC=$(brew --prefix llvm)/bin/clang
export CXX=$(brew --prefix llvm)/bin/clang++
cmake -B cmake-build-debug -S . -DCMAKE_BUILD_TYPE=Debug -G Ninja

# Build
cmake --build cmake-build-debug

# Run
./cmake-build-debug/airborne
```

Dependencies (fetched automatically via CMake FetchContent): raylib 5.5, EnTT v3.16.0, nlohmann_json v3.11.3.

There are no tests, linters, or CI pipelines.

## Architecture

### Screen State Machine

`main.cpp` runs a screen state machine: `SPLASH → LOADING → GAMEPLAY`. Each screen extends `BaseScreen` (virtual `update()` / `draw()`). The `update()` return value triggers transitions. A single `entt::registry` is created in `main()` and passed to screens that need it.

### ECS (EnTT)

Everything flows through a single `entt::registry`:

- **Components** (`src/components/`) — plain structs, no methods. Split into three module partitions:
  - `:World` — gameplay components (`Player`, `Aircraft`, `Position3D`, `Rotation`, `Orientation`, `LandingZone`, `Forces`, etc.)
  - `:Data` — player state and global data (`PlayerEntity`, `PlayerInputs`, `GroundHeight`, `Offset`)
  - `:Render` — resource handles and widget configs (`WithModel`, `WithTexture`, `HudWidget`, `RadarWidget`, etc.)
- **Systems** (`src/systems/`) — free functions named in `PascalCase` (e.g., `RenderModels`, `WorldStreamerSystem`, `RenderCockpit`). Systems query the registry using `registry.view<...>()`.
- **Prefabs** (`src/prefabs/`) — factory functions in the `factories` namespace that create entities with component bundles (e.g., `create_player`, `create_unit`, `create_scene`).

### Player Pipeline

The player update is orchestrated by `PlayerDispatcher`, which chains subsystems in order:

`PlayerControls → PlayerPhysics → PlayerPosition → PlayerRotation → PlayerCamera → PlayerGroundCheck`

Each subsystem is a class with a config (loaded from JSON) and an `update(registry, dt)` method that queries the registry for `Player` + `PlayerInputs` components.

### Game Loop

`Game` class (`src/game.cppm`) owns the scenario and player dispatcher. Each frame:
1. `update()`: dispatcher pipeline + widget inputs (if not crashed)
2. `draw()`: 3D pass (world streamer, models, debug) → 2D pass (cockpit, minimap, HUD, radar, crash layout)

### Configuration

All game configuration lives in JSONC files under `assets/`:
- `main.jsonc` — global window/render settings
- `config.jsonc` — player aircraft, cockpit, HUD, radar, scene configs
- `scenario.jsonc` — mission definition (start conditions, entity spawns)
- `resources.jsonc` — resource manifest for preloading models, textures, sounds

Configs are loaded via `JsonConfig` (wrapper around nlohmann_json) using JSON pointer paths (e.g., `config.get<PlayerPhysicsConfig>("/player/aircraft")`). Config types use `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE` macros for automatic serialization.

### Entity Spawning

Scenario entities are defined in `scenario.jsonc` as `EntityDef` structs. The `create_unit` factory maps each def to ECS components: `Identify`, `IdentifyType`, `Position3D`, `Heading`, `FriendFoe`, and optionally `WithModel`, `Landable`, `Carrier`. See `scenario.md` for the full entity type specification.

## C++ Conventions

### Module Structure

Every `.cppm` file follows this layout:
```c++
module;
// #include directives only here (before export)
#include <vector>

export module MyModule;

// import directives only here (after export)
import JsonConfig;

export class MyClass { ... };
```

Module partitions use the colon syntax (e.g., `export module Components:World;`, imported via `export import :World;`).

### Naming

- `PascalCase` — classes, structs, enums, and system functions
- `snake_case` — variables, methods, member functions, factory functions
- System free functions use `PascalCase` to distinguish them from regular functions

### Modern C++ Style

- `static_cast` (never C-style casts)
- `if-init` statements where applicable
- `auto` for type inference
- `constexpr` for compile-time constants
- `nullptr` (never `NULL`)
- Semantic type aliases for units: `Meter`, `Pixel`, `Newton`, `AngleDeg`, `AngleRad`, `MeterPerSecond`, etc. (defined in `Types:Units`)

### raylib Wrapping

raylib headers are included via `src/lib/ray.hpp` (not directly). The project uses raylib's `Vector3`, `Quaternion`, `Color`, `Camera`, `Model`, `Texture`, `Shader` types throughout.

## Key Files

- `src/main.cpp` — entry point, screen state machine
- `src/game.cppm` — game loop orchestration
- `src/player/player-dispatcher.cppm` — player update pipeline
- `src/components/` — all ECS components
- `src/prefabs/create-unit.cppm` — entity factory (see `scenario.md` for entity types)
- `plan.md` — active ECS migration plan (legacy OOP → full ECS)
