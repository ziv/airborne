# Copilot Instructions — Airborne

A modern C++23 remake of F-15 Strike Eagle II using raylib, EnTT (ECS), and nlohmann/json. Not a full simulator, not an arcade — a gameplay-focused flight game.

## Build

Requires LLVM/Clang with C++20 modules support, CMake 4.2+, and Ninja.

```shell
# Configure (one-time)
export CC=$(brew --prefix llvm)/bin/clang
export CXX=$(brew --prefix llvm)/bin/clang++
cmake -B cmake-build-debug -S . -DCMAKE_BUILD_TYPE=Debug -G Ninja

# Build
cmake --build cmake-build-debug

# Run
./cmake-build-debug/airborne
```

Dependencies (raylib 5.5, EnTT 3.16, nlohmann/json 3.11) are fetched automatically via CMake `FetchContent`. No tests or linter exist.

## Architecture

### Game loop

`main.cpp` → screen state machine (`SplashScreen`, `GameScreen`). `GameScreen` owns an `entt::registry` and a `Game` instance. `Game` orchestrates the frame:

- **Update**: `PlayerDispatcher` runs a pipeline of player subsystems (controls → physics → position → rotation → camera → ground-check), then `WidgetsInputs` handles cockpit widget toggling.
- **Draw**: 3D pass (world streamer, models, debug) inside `BeginMode3D`, then 2D pass (cockpit, minimap, engine status, HUD, radar, crash layout).

### ECS (EnTT)

The project uses EnTT as its entity-component-system. Key patterns:

- **Player entity** is stored in `registry.ctx()` as `PlayerEntity{id}`. Access it via `get_player_entity(registry)` from `Accessors`.
- **Components** are plain structs in `src/components/` split into three module partitions: `World` (gameplay components), `Data` (player state, inputs, tags), `Render` (resource handles, widget configs).
- **Tag components** like `Grounded`, `Flying`, `Crashed`, `TouchDown`, `Autopilot` represent states — added/removed rather than mutated.
- **Global state** uses `registry.ctx()`: `Offset` (large-world offset), `Forces` (debug), `ResourceManager`.
- **Prefabs** (`src/prefabs/`) are factory functions in the `factories` namespace that create entities with their component sets.
- **Systems** are either free functions (renderers, world streamer) or classes with an `update(registry, dt)` method (player subsystems).

### Resource management

`ResourceManager` is stored in registry context and holds `entt::resource_cache` instances for textures, models, images, shaders, and music streams. Resources are preloaded at startup from `assets/resources.jsonc`.

### Configuration

All config is loaded from JSONC files via `JsonConfig` (wrapper around nlohmann/json with JSON pointer paths):
- `assets/config.jsonc` — window, player physics, camera, HUD layout, cockpit, etc.
- `assets/scenario.jsonc` — mission definition (start conditions, entities, theater).
- `assets/resources.jsonc` — resource paths for preloading.

Config structs use `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE` for automatic serialization.

### Large-world offset

The world uses a re-centering strategy: `Offset` (in registry context) shifts all world-space positions so the player stays near the origin, avoiding floating-point precision issues at large coordinates.

## C++ module structure

All source files use C++23 modules (`.cppm` extension). The canonical module layout:

```cpp
module;
#include <...>          // Only #includes go in the global module fragment

export module MyModule; // Or: export module Parent:Partition;

import OtherModule;     // Only imports go here

export class/struct/function ...
```

`#include` directives must only appear in the global module fragment (before `export module`). `import` statements must come after `export module`. Aggregate modules re-export partitions: `export import :Partition;`.

All modules must be registered in `CMakeLists.txt` under `FILE_SET CXX_MODULES`.

## Conventions

- **Naming**: `PascalCase` for classes, structs, enums. `snake_case` for variables, methods, functions. System functions called from the game loop use `PascalCase` (e.g., `RenderCockpit`, `WorldStreamerSystem`).
- **Unit type aliases**: Use semantic aliases from `units-types.cppm` — `Meter`, `Knot`, `Pixel`, `AngleDeg`, `Newton`, etc. — not raw `float`/`int`.
- **No game engine**: This project intentionally uses raylib directly, not a game engine.
- **Components are data-only structs**: No methods, no inheritance.
- **Coordinate system**: raylib's right-handed Y-up. Forward is +Z, up is +Y, right is -X (see `helpers.cppm`).
- **raylib wrapper**: `src/lib/ray.hpp` is used instead of including raylib headers directly in module files.

## Active migration

The project is undergoing an ECS migration (see `plan.md` at repo root). The player subsystems are currently classes in `src/player/` that operate on `Player` and `PlayerInputs` components. The plan is to convert them to free-function ECS systems and migrate remaining OOP patterns to pure ECS.
