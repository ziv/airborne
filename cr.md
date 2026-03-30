# Code Review — Airborne (Flight Simulator)

> Reviewed: 2026-03-29 | Stage: Early development  
> Stack: C++17, raylib 5.5, CMake, nlohmann/json

---

## Overall Impression

Solid foundation for an early-stage project. The architecture is clean — screen state machine,
quaternion-based flight model, data-driven config via JSON, and a good separation into
primitives / items / views / utils / shaders. The flight physics (thrust, lift, drag, gravity,
weathervaning) are a compelling arcade-sim blend, exactly matching the stated goal.

The items below are ordered roughly by severity.

---

## 🐛 Bugs

## ⚠️ Design & Architecture Issues

### 5. Dual source of truth: `Constants.h` vs `app.json`

`Constants.h` defines `SCREEN_WIDTH`, `MAX_SPEED`, `PITCH_RATIO`, etc. as compile-time constants.
`app.json` (loaded by `AppConfig`) defines the same values at runtime. Code uses both —
`Hud.h` reads from `Constants.h`, while `GameplayScreen` reads from `AppConfig`.
Pick one authoritative source; `app.json` is the better choice since it's already data-driven.

### 6. `AppConfig` copied into `GameData` by value

```cpp
// GameData.h:40
AppConfig config;   // value — copies entire JSON blob
```

`GameScreen` already holds `AppConfig&` by reference. `GameData` should do the same to avoid
copying and stay in sync.

### 7. Fragile resource loading in member initializers (`GameplayScreen.h:17-19`)

```cpp
Texture2D cockpit = LoadTexture("res/cockpit-g1-cut.png");
Shader chromaShader = LoadShader(nullptr, "src/shaders/chromakey.fs");
Music engine = LoadMusicStream("res/engine.mp3");
```

These call raylib before the constructor body runs. They work today only because the screen is
constructed after `InitWindow`, but this is implicit and fragile. Move them into the constructor
body.

### 8. No copy/move guards on resource-owning classes

`SplashScreen`, `GameplayScreen`, `Aircraft` all own raylib resources (textures, models, music)
that are freed in destructors. An accidental copy would cause double-free. Add:
```cpp
SplashScreen(const SplashScreen&) = delete;
SplashScreen& operator=(const SplashScreen&) = delete;
```

### 9. Velocity clamping loop (`GameData.cpp:72-74`)

```cpp
while (Vector3Length(velocity) > config.maxSpeed()) {
    velocity = Vector3Scale(velocity, 0.9f);
}
```

This is O(n) with potentially many iterations. Replace with a single normalization:
```cpp
float speed = Vector3Length(velocity);
if (speed > config.maxSpeed()) {
    velocity = Vector3Scale(velocity, config.maxSpeed() / speed);
}
```

### 10. `SplashScreen` skips the main menu

`SplashScreen::Update()` transitions straight to `GAMEPLAY`. `MainMenuScreen` is unreachable
in the current flow. Intentional? If so, the main menu code is dead weight.

---

## 🧹 Code Hygiene

### 11. Spelling errors in identifiers

| Current | Should be |
|---------|-----------|
| `breaks` (GameData.h, GameplayScreen.cpp) | `brakes` |
| `Crushing` / `Crushed` (PlaneState enum) | `Crashing` / `Crashed` |
| `rollRaio` (app.json + AppConfig.h) | `rollRatio` |
| `clipPlans` (app.json + AppConfig.h) | `clipPlanes` |
| `"BANK NAD ROLL"` (Autopilot.cpp:46 comment) | `"BANK AND ROLL"` |

### 12. Excessive commented-out code

`GameplayScreen.cpp`, `Utils.h`, `Aircraft.cpp`, `Constants.h`, and `main.cpp` all carry large
blocks of dead commented code. This should be removed — git history preserves it if needed.

### 13. Non-English comments

`PowerGauge.h` and `chromakey.fs` contain Hebrew comments. For a shareable codebase, translate to English.

### 14. Duplicate utility functions

`Utils.h` contains `LoadAppConfig()` which duplicates `AppConfig::AppConfig()`.
`TmpLoadModel()` duplicates `UtilsLoaders::loadTerrain()`. Remove the dead copies.

### 15. `GetFlatForward` doesn't normalize its output (`Utils.h:15-22`)

The Y component is zeroed but the result isn't normalized, so callers receive a non-unit vector.
This affects heading calculations in the autopilot.

### 16. `gravity` lives in `Utils.h` (line 68)

A physics constant hiding in a utility header. Move to `Constants.h` or a `Physics.h`.

### 17. Magic literal `10.0f` for ground level

`GameData.cpp:80-82` and `GameplayScreen.cpp:18` both hardcode the ground altitude as `10.0f`.
Extract to a named constant.

### 18. Missing `<vector>` include in `Autopilot.h`

The class uses `std::vector<Waypoint>` but never directly includes `<vector>`. It compiles
through transitive includes, which is fragile.

### 19. `Aircraft::model` is public

```cpp
class Aircraft {
    std::string_view name;   // private
public:
    Model model;             // exposed — breaks encapsulation
```

Also, storing the name as `string_view` is dangerous if the source string is a temporary.

### 20. Empty `Structure` class

`Structure.h/.cpp` is a skeleton with an empty `Draw()`. Either implement or remove.

### 21. `game.json` is unused

Contains only `{ "version": "0.0.0" }` — no code reads it.

### 22. Hardcoded resource paths

`GameplayScreen.h` hardcodes `"res/cockpit-g1-cut.png"`, `"src/shaders/chromakey.fs"`,
`"res/engine.mp3"`, `"res/mig-29.glb"`. These should come from `app.json` for consistency.

---

## 💡 Suggestions (Non-blocking)

- **Ground collision handling**: Currently the plane just clamps to `y=10`. As the sim grows,
  consider a proper state transition to `Crashing`/`Crashed`.
- **Engine sound on autopilot**: The engine pitch/volume update only runs in manual mode.
  The autopilot path (`GameplayScreen::Update()` line 60) returns early before updating audio.
- **Screen resolution for HUD**: Consider a virtual resolution / scaling system so HUD
  elements stay proportional regardless of window size.
- **Build instructions**: The README says `cmake --build cmake-build-debug` but doesn't show
  the initial `cmake -B cmake-build-debug` configure step.

---

*Nice project — the MicroProse spirit is alive. Happy flying 🛩️*
