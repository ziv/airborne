# ECS Migration Plan

## Current Architecture Summary

The codebase is a raylib-based flight simulator (F-15 Strike Eagle II remake) with two parallel entity systems:

### 1. Legacy OOP Entity System (active, used in gameplay)
- `EntityBase` — virtual base class with `draw()` / `update()` overrides
- `EntityRegistry` — singleton (`EntityRegistry::get()`) owning `vector<unique_ptr<EntityBase>>`
- Subclasses: `Structure`, `Aaa`, `Airbase`, `Carrier` (each override draw/update)
- `EntityDef` — fat data struct (id, type, faction, state, position, heading, health, model, params)
- `GroundTarget` extends `EntityDef` with damage logic

### 2. Partially Started ECS (EnTT, mostly unused)
- `entt::registry` created in `main.cpp`, player entity created via `Factory::createPlayer`
- Components defined: `components/aircraft.h` (`Aircraft`, `Aerodynamics`, `Engine`, `Hud`) and `components/world.h` (`Player`, `View3D`, `Position3D`, `Velocity`, `Rotation`, `Orientation`, `RigidBody`)
- Systems defined: `systems/physics.h` (`ApplyAircraftPhysics`) and `systems/hud.h` (`DrawHud`)
- The `entt::registry` is never passed to `GameplayScreen` or `GameData` — it's dead code

### 3. Player Aircraft System (active, monolithic)
- `AircraftState` — single mutable struct passed through the pipeline
- `GameData` orchestrates: `AircraftControls` → `AircraftPhysics` → `AircraftTransformation` → `AircraftCamera`
- Each subsystem reads/writes `AircraftState` directly
- `GameplayScreen` owns `GameData`, `SceneManager`, cockpit views
- Views (`CockpitView`, `DebugView`, `MinihudView`, `MapView`, `RadarView`, `EngineView`) all read `AircraftState`

### Key Pain Points
1. **Two entity systems** — OOP entities and ECS components exist side-by-side but don't talk
2. **God struct** — `AircraftState` is a monolith (~30 fields) passed everywhere by reference
3. **Singleton registry** — `EntityRegistry::get()` is called from views, screens, and GameData
4. **No shared registry** — the `entt::registry` in `main.cpp` is never passed into gameplay
5. **Tight coupling** — views depend on `GameData&` or `AircraftState` directly

---

## Migration Strategy

The migration is incremental. Each phase produces a compilable, runnable build. No big rewrites.

### Phase 0 — Plumbing: Pass the Registry Through

**Goal:** Make `entt::registry` available everywhere the old systems are used.

1. Move `entt::registry` creation from `main.cpp` into `GameplayScreen` (or pass it in)
2. Add `entt::registry&` to `GameData` constructor
3. Store the registry reference in `GameData` as a member
4. Pass `entt::registry&` to `GameplayScreen`'s subsystems that need it

**Files changed:** `main.cpp`, `GameData.h/.cpp`, `GameplayScreen.h/.cpp`

**Validation:** Build + run. Behavior is unchanged — registry exists but is just carried around.

---

### Phase 1 — Player Entity on the Registry

**Goal:** The player aircraft is an EnTT entity with components. `AircraftState` still exists but reads/writes go through components.

#### 1a. Define final component set

Keep the components in `components/world.h` and `components/aircraft.h` but refine them to mirror what `AircraftState` holds today:

```
// components/world.h (already exists, keep as-is)
Position3D, Velocity, Rotation, Orientation, Player, View3D

// components/aircraft.h — extend
Aircraft       { bool gear; bool brakes; }          // already exists
Aerodynamics   { ... }                               // already exists
Engine         { float maxThrust; float currentThrottle; }  // already exists

// NEW components
PilotControls  { float pitch, yaw, roll, throttle; bool brakes, gear; }
PhysicsState   { Newton thrust, drag, lift; MeterPerSecond speed; Vector3 velocity; }
MapOffset      { Vector2 offset; }
FlightState    { bool flying; bool crashed; float fuel; float groundHeight; float effectiveFloorHeight; }
LandingZone    { bool active; bool isCarrier; float surfaceHeight; }
AutopilotState { bool active; int waypointIndex; }
```

#### 1b. Create player entity with all components

Expand `Factory::createPlayer` (in `prefabs/PlayerCreator.h`) to emplace all components matching current `AircraftState` initial values.

#### 1c. Bridge: AircraftState ↔ Components

Add a temporary sync layer:
- `syncStateFromComponents(registry, entity, AircraftState&)` — copies component data → AircraftState
- `syncComponentsFromState(registry, entity, const AircraftState&)` — copies AircraftState → components

This allows existing subsystems (`AircraftPhysics`, `AircraftControls`, etc.) to keep working on `AircraftState` while components are the source of truth.

**Files changed:** `components/aircraft.h`, `components/world.h`, `prefabs/PlayerCreator.h`, new `systems/sync.h`

**Validation:** Build + run. Player flies normally. Components are kept in sync but nothing reads them yet.

---

### Phase 2 — Convert Subsystems to ECS Systems

**Goal:** Replace class-based subsystems with free functions operating on component views. One at a time.

#### Conversion order (least coupled → most coupled):

**2a. AircraftControls → InputSystem**
```cpp
// systems/input.h
void ProcessInput(entt::registry& reg, float dt);
```
- Query `view<PilotControls, FlightState, Player>`
- Copy keyboard logic from `AircraftControls::update`
- Remove `AircraftControls` class

**2b. AircraftPhysics → PhysicsSystem**
```cpp
// systems/physics.h (replace current stub)
void ApplyPhysics(entt::registry& reg, float dt);
```
- Query `view<Position3D, PhysicsState, PilotControls, Orientation, FlightState, MapOffset>`
- Copy force logic from `AircraftPhysics::update`
- Include large-world re-centering
- Remove `AircraftPhysics` class

**2c. AircraftTransformation → OrientationSystem**
```cpp
// systems/orientation.h
void UpdateOrientation(entt::registry& reg, float dt);
```
- Query `view<Orientation, Rotation, PilotControls, PhysicsState, FlightState>`
- Copy flying/ground orientation from `AircraftTransformation`
- Remove `AircraftTransformation` class

**2d. AircraftCamera → CameraSystem**
```cpp
// systems/camera.h
void UpdateCamera(entt::registry& reg, Camera& camera, float dt);
```
- Query `view<Position3D, Orientation, Player>`
- Copy camera logic from `AircraftCamera`
- Remove `AircraftCamera` class

**2e. Autopilot → AutopilotSystem**
```cpp
// systems/autopilot.h
void RunAutopilot(entt::registry& reg, float dt);
```
- Query entities with `AutopilotState` + `PilotControls`
- Copy waypoint steering from `Autopilot`
- Remove `Autopilot` class

After each subsystem conversion:
- Remove the corresponding sync bridge calls
- Verify build + run

**Files changed/created:** `systems/input.h`, `systems/physics.h`, `systems/orientation.h`, `systems/camera.h`, `systems/autopilot.h`
**Files removed:** `core/AircraftControls.h/.cpp`, `core/AircraftPhysics.h/.cpp`, `core/AircraftTransformation.h/.cpp`, `core/AircraftCamera.h/.cpp`, `core/Autopilot.h/.cpp`

**Validation:** After all five, the player aircraft runs entirely through ECS systems. `AircraftState` struct can be deleted. `GameData` becomes a thin orchestrator that calls systems in order.

---

### Phase 3 — Migrate World Entities to ECS

**Goal:** Replace `EntityRegistry` singleton + OOP hierarchy with EnTT entities.

#### 3a. Define world entity components

```
// components/entity.h
EntityInfo     { EntityId id; EntityType type; std::string subtype; Faction faction; EntityState state; }
Health         { float current; float max; }
ModelRef       { std::string modelId; }
Heading        { float degrees; }
Scale          { float value; }
LandingBox     { float width; float length; float height; float radiusSquared; }
```

Reuse existing `Position3D` for entity positions.

#### 3b. Entity spawning

Replace `EntityRegistry::spawn()` with a factory that creates EnTT entities:
```cpp
// prefabs/EntityCreator.h
entt::entity spawnEntity(entt::registry& reg, const EntityDef& def);
```
- Emplace `EntityInfo`, `Position3D`, `Health`, `Heading`, `ModelRef`, `Scale`
- For `AIRBASE`/`CARRIER`: also emplace `LandingBox`
- Model loading stays centralized (a model cache resource, not a component)

#### 3c. Convert entity update logic

**LandingZoneSystem:**
```cpp
void CheckLandingZones(entt::registry& reg, float dt);
```
- Query `view<LandingBox, Position3D, Heading, EntityInfo>` where faction == FRIENDLY
- Check player distance + box intersection → write player's `LandingZone` component
- Replaces `Airbase::update`, `Carrier::update`, and `GameData::checkLandingZones`

**DamageSystem:**
```cpp
void ApplyDamage(entt::registry& reg);
```
- Query `view<Health, EntityInfo>` for damage events
- Replaces `GroundTarget::takeDamage`

#### 3d. Convert entity rendering

**EntityRenderSystem:**
```cpp
void DrawEntities(entt::registry& reg, const Vector3& playerPos, const Vector2& mapOffset);
```
- Query `view<Position3D, Heading, ModelRef, EntityInfo, Scale>` where state is alive
- Draw model or fallback cube
- Replaces `EntityRegistry::draw` and all `EntityBase::draw` overrides

#### 3e. Convert radar view to query registry

`RadarView::draw` currently calls `EntityRegistry::get().forEachAlive(...)`. Change it to accept `entt::registry&` and query `view<EntityInfo, Position3D, Heading>`.

#### 3f. Remove old entity system

- Delete `EntityRegistry` singleton class
- Delete `EntityBase`, `Structure`, `Aaa`, `Airbase`, `Carrier` classes
- Delete `GroundTarget` struct
- `EntityDef` survives as a deserialization-only struct for scenario loading

**Files created:** `components/entity.h`, `prefabs/EntityCreator.h`, `systems/landing_zone.h`, `systems/damage.h`, `systems/entity_render.h`
**Files removed:** `entities/EntityRegistry.h/.cpp`, `entities/Entity.h/.cpp`, `entities/Structure.h/.cpp`, `entities/Aaa.h/.cpp`, `entities/Airbase.h/.cpp`, `entities/Carrier.h/.cpp`, `entities/GroundTarget.h/.cpp`, `entities/EntitiesUtils.h`

**Validation:** All entities spawn, draw, and behave as before. Radar shows contacts. Landing zones work.

---

### Phase 4 — Decouple Views from GameData

**Goal:** Views read components from the registry instead of `AircraftState` or `GameData&`.

#### 4a. Pass `const entt::registry&` to views

Change view constructors/draw/update signatures:
- `CockpitView`, `MinihudView`, `MapView`, `RadarView`, `EngineView`, `OffView`, `DebugView`, `NavballView`
- Each queries the player entity (has `Player` tag) and reads its components

#### 4b. Remove GameData& from views

- `DebugView` currently takes `GameData&` — change to `const entt::registry&`
- `CockpitView` takes `GameData&` for the pane system — change to `const entt::registry&`

**Files changed:** All view headers/sources

**Validation:** Build + run. All HUD elements display correctly.

---

### Phase 5 — Simplify GameData / Game Loop

**Goal:** `GameData` becomes a thin system orchestrator or disappears entirely.

1. `GameData` no longer owns subsystem classes (they're gone after Phase 2)
2. Its `update()` becomes a sequence of system calls:
   ```cpp
   void GameData::update(entt::registry& reg, float dt) {
       CheckLandingZones(reg, dt);
       ProcessInput(reg, dt);     // or RunAutopilot
       ApplyPhysics(reg, dt);
       UpdateOrientation(reg, dt);
       UpdateCamera(reg, camera, dt);
       DrawEntities(reg, ...);
   }
   ```
3. Consider inlining this into `GameplayScreen::update()` and removing `GameData` entirely

**Files changed:** `GameData.h/.cpp`, `GameplayScreen.h/.cpp`
**Files potentially removed:** `GameData.h/.cpp`, `core/AircraftStructs.h` (split into components)

---

### Phase 6 — Cleanup

1. Delete `AircraftState` struct (all data lives in components now)
2. Delete `AircraftStructs.h` (replaced by component headers)
3. Delete the sync bridge (`systems/sync.h`) if still present
4. Remove the unused `entt::registry` + `Factory::createPlayer` from `main.cpp` (registry now lives in GameplayScreen)
5. Clean up `#include` chains — many headers pulled `AircraftStructs.h` transitively
6. Delete commented-out code throughout the codebase

---

## File Structure After Migration

```
src/
  components/
    aircraft.h        # Aircraft, Aerodynamics, Engine, PilotControls, PhysicsState, FlightState, etc.
    world.h           # Player, Position3D, Velocity, Rotation, Orientation, MapOffset, View3D
    entity.h          # EntityInfo, Health, ModelRef, Heading, Scale, LandingBox
  systems/
    input.h           # ProcessInput
    physics.h         # ApplyPhysics
    orientation.h     # UpdateOrientation
    camera.h          # UpdateCamera
    autopilot.h       # RunAutopilot
    landing_zone.h    # CheckLandingZones
    damage.h          # ApplyDamage
    entity_render.h   # DrawEntities
    hud.h             # DrawHud (already started)
  prefabs/
    PlayerCreator.h   # createPlayer (expanded)
    EntityCreator.h   # spawnEntity (new)
  views/              # unchanged structure, but take const registry& instead of AircraftState
  scenario/           # unchanged (data loading only)
  primitives/         # unchanged (types, config, utils, resources)
  core/
    SceneManager.h/.cpp  # unchanged (terrain, fog, clouds, audio)
  main.cpp
  GameplayScreen.h/.cpp  # owns registry, calls systems in order
  GameScreen.h           # base screen class (unchanged)
  ...Screen files        # unchanged
```

---

## Migration Order Checklist

| #  | Task | Risk | Dependencies |
|----|------|------|--------------|
| 0  | Pass registry through GameplayScreen → GameData | Low  | None |
| 1a | Refine component definitions | Low  | Phase 0 |
| 1b | Expand PlayerCreator with all components | Low  | 1a |
| 1c | Build sync bridge (AircraftState ↔ components) | Med  | 1b |
| 2a | Convert AircraftControls → InputSystem | Low  | 1c |
| 2b | Convert AircraftPhysics → PhysicsSystem | Med  | 2a |
| 2c | Convert AircraftTransformation → OrientationSystem | Med  | 2b |
| 2d | Convert AircraftCamera → CameraSystem | Low  | 2c |
| 2e | Convert Autopilot → AutopilotSystem | Low  | 2a |
| 3a | Define world entity components | Low  | Phase 0 |
| 3b | Entity spawning factory | Low  | 3a |
| 3c | LandingZoneSystem + DamageSystem | Med  | 3b |
| 3d | EntityRenderSystem | Low  | 3b |
| 3e | Convert RadarView to query registry | Low  | 3b |
| 3f | Delete old EntityRegistry + OOP classes | Low  | 3c, 3d, 3e |
| 4  | Decouple views from GameData | Med  | Phase 2, Phase 3 |
| 5  | Simplify/remove GameData | Low  | Phase 4 |
| 6  | Final cleanup | Low  | Phase 5 |

Phases 2 and 3 can proceed in parallel since they touch different parts of the code.

---

## Principles

- **One subsystem at a time.** Each conversion is a single PR/commit that compiles and runs.
- **Sync bridge first.** The bridge lets old code and new code coexist during transition.
- **Components are plain structs.** No methods, no inheritance, no virtuals.
- **Systems are free functions.** No classes wrapping a single `update()` method.
- **EnTT views, not manual loops.** Query the registry for the component combination you need.
- **Keep EntityDef for deserialization only.** Scenarios define entities as JSON → `EntityDef` → factory creates EnTT entities with proper components.
