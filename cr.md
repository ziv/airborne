# Code Review — Airborne

> Reviewed: all source under `src/`, `CMakeLists.txt`, `app.json`, and `shaders/`.
> The project is early-stage; this review focuses on what's already written, not on missing features.

---

## 🐛 Bugs

### 1. `Types.h` — `operator""_deg` returns `Newton` instead of `Degree`

```cpp
// line 30
constexpr Degree operator""_deg(const long double val) {
    return static_cast<Newton>(val);  // ← should be static_cast<Degree>(val)
}
```

### 2. `AppConfig.h` — `showGrid` declared as `float`, should be `bool`

```cpp
// line 55
float showGrid = true;  // ← should be: bool showGrid = true;
```

### 3. `shaders/map.fs` — won't compile

- `texelColor` is declared twice (lines 12 and 24).
- `glareColor` is referenced (line 33) but the declaration is commented out.

### 4. `GameplayScreen.h` — `futuristicCity` loaded at member-init time

```cpp
Model futuristicCity = LoadModel("res/futuristic_city.glb");
```

`LoadModel` calls into OpenGL. When a `GameplayScreen` is constructed, raylib's window already exists
so it *happens* to work, but:

- It bypasses the config-driven path pattern used everywhere else — the path is hardcoded.
- It's **never unloaded** in the destructor (memory/GPU leak).

### 5. `MapView.h` — same member-init-time loading issue

```cpp
Texture2D map = LoadTexture("res/map.png");
Shader glassShader = LoadShader(nullptr, "glass_hud.fs");
int timeLoc = GetShaderLocation(glassShader, "time");
```

Hardcoded paths, and the shader/texture are never loaded through `AppConfig`. The shader path
`"glass_hud.fs"` doesn't match any file in `shaders/` (the actual file is `shaders/map.fs`).

### 6. `GameData` — hardcoded ground altitude `10` / `10.0f` in multiple places

- `applyForces()` line 96: `if (camera.position.y <= 10 && …)` — should be `config.heightAboveGround`.
- `applyPosition()` line 217–220: `if (newPosition.y <= 10.0f) … newPosition.y = 10.0f;` — same.

`config.heightAboveGround` is `3` in `app.json`, so these checks are inconsistent with each other
and with the rest of the code that uses the config value.

### 7. `AppConfig` — `gameMapMinimap` declared but never loaded

The field exists in the header but is never assigned in the constructor.

### 8. Dual autopilot state

`GameData` has its own `bool autoPiloting` field, while `Autopilot` has a separate `bool active`.
`GameData::toggleAutopilot()` toggles one; `Autopilot::Toggle()` toggles the other.
`GameplayScreen` only uses the `Autopilot` one. The `GameData` flag is dead code waiting to cause a
bug.

---

## 🏗️ Architecture & Design

### 9. `GameData` is a god object

`GameData` currently owns the camera, the physics, the rotation state, the velocity, the controls,
and the display dimensions. As the project grows, consider splitting it:

| Concern | Potential class |
|---------|----------------|
| Aircraft state (position, rotation, velocity) | `AircraftState` / `Transform` |
| Physics simulation | `PhysicsEngine` |
| Camera management | `CameraController` |
| Player input / controls | `ControlState` |
| Game-level bookkeeping (pause, delta, screen size) | Keep in `GameData` |

This also makes unit-testing physics in isolation trivial.

### 10. `View` base class — `config` is private

```cpp
class View {
    AppConfig &config;  // ← private, derived classes can't access it
```

Derived views (`HudView`, `GaugesView`, etc.) never use `config` through the base class because
they can't. Either make it `protected` or remove it and pass config where needed.

### 11. `GameScreen` includes `json.hpp` for no reason

`GameScreen.h` includes `lib/json.hpp` and has `using json = nlohmann::json;`. Neither the base
class nor most derived classes need JSON. This adds ~25K lines to every translation unit that
includes `GameScreen.h`. Move the JSON include to the `.cpp` files that actually need it.

### 12. Screen state machine doesn't handle `EXIT`

The `ScreenState` enum has an `EXIT` value, but the `switch` in `main.cpp` never handles it — the
`default` case creates a new `SplashScreen`. A screen returning `EXIT` should break the game loop.

### 13. Unscoped enums

`AircraftState` and `GearState` are plain `enum`, not `enum class`. Values like `Ground`, `Opened`,
`Closed` pollute the enclosing namespace and risk name collisions.

```cpp
// Prefer:
enum class AircraftState { Ground, Flying, Crashed };
enum class GearState     { Retracted, Extended };
```

(Also: `Crushed` → `Crashed` — typo.)

---

## 📖 Readability

### 14. Inconsistent naming convention

| Location | Style | Example |
|----------|-------|---------|
| `Autopilot` methods | PascalCase | `Toggle()`, `AddWaypoint()`, `Steer()` |
| `GameData` methods | camelCase | `applyForces()`, `recalcVectors()` |
| `PilotControls` fields | PascalCase | `Pitch`, `Roll`, `Yaw` |
| `GameData` fields | camelCase | `throttle`, `speed` |

Pick one convention and stick with it. For C++ game code, `camelCase` for methods and `camelCase`
for fields is most common; the Unreal convention is `PascalCase` everywhere. Either is fine —
just be consistent.

### 15. Cryptic variable names in `GameplayScreen.cpp`

```cpp
constexpr Vector3 l1 = {5000.0f, 1500.0f, 5000};
constexpr Vector3 l2 = {9000.0f, 1500.0f, 4000.0f};
constexpr Vector3 l3 = {10000.0f, 1500.0f, 8000.0f};
constexpr Vector3 l4 = {5500.0f, 1500.0f, 6500.0f};
constexpr Vector3 a{0.0f, 10.0f, 0.0f};
```

These are waypoint locations with an altitude offset. Names like `waypointBase1` (or put them in an
array) and `altitudeOffset` would be self-documenting.

### 16. Magic numbers scattered throughout

A non-exhaustive list:

| File | Line(s) | Magic number |
|------|---------|-------------|
| `GaugesView.cpp` | 36–37, 83–84 | `370`, `460`, `406`, `64`, `30` (pixel positions) |
| `MapView.cpp` | 19–20 | `64.0f` (meters-per-pixel) |
| `MapView.cpp` | 24 | `522.0f + 79.0f`, `542.0f + 67.0f` |
| `MapView.cpp` | 37 | `522, 542, 158, 134` |
| `GameData.cpp` | 144 | `1000` (brake multiplier), `0.9f` |
| `GameData.cpp` | 147 | `0.1` (ground lift factor) |
| `GameData.cpp` | 205 | `6.0` (air-brake drag multiplier) |
| `GameData.cpp` | 208 | `1.8f` (gear drag multiplier) |
| `GameData.cpp` | 211 | `0.1` (stall lift factor) |
| `Autopilot.cpp` | 64 | `1.5f` (heading error gain) |
| `Autopilot.cpp` | 75 | `2.0f` (roll gain) |

Consider putting physics constants in the config JSON or as named `constexpr` values.

### 17. Spelling

- `breaks` → `brakes` (throughout the codebase)
- `Crushed` → `Crashed` (`GameData.h`)
- `clipPlans` → `clipPlanes` (`AppConfig.h`)
- `fround` → `ground` (comment in `GameData.cpp:84`)
- `colision` → `collision` (comment in `GameData.cpp:22`)

---

## ⚡ Performance

### 18. `GaugesView::drawPower()` — excessive repetition

The entire function is ~50 lines of nearly identical if/else blocks. This can be reduced to a
small loop:

```cpp
void GaugesView::drawPower(const GameData &game) {
    constexpr int x = 370, y = 460;
    struct GaugeStep { float threshold; const char* color; };
    constexpr GaugeStep steps[] = {
        {1.0f, "power-gauge-red"},
        {0.8f, "power-gauge-yellow"},
        {0.6f, "power-gauge-yellow"},
        {0.4f, "power-gauge-green"},
        {0.2f, "power-gauge-green"},
    };
    for (int i = 0; i < 5; i++) {
        const char* sprite = (game.throttle >= steps[i].threshold)
            ? steps[i].color : "power-gauge-off";
        drawSprite(sprite, {(float)x, (float)(y - i * 15)});
    }
    // ... label
}
```

### 19. `GaugesView::drawSprite()` — string-keyed map lookup every frame

```cpp
void GaugesView::drawSprite(const std::string &name, …) {
    if (const auto it = map.find(name); …)
```

Every call constructs a `std::string` from the literal and hashes it. For a small fixed set of
sprites, use an `enum` key or `std::string_view`-compatible lookup
(`std::unordered_map` doesn't support heterogeneous lookup, but a sorted `std::vector<pair>` with
binary search on `string_view` would, or switch to a flat array indexed by enum).

### 20. `FormatNumber()` allocates every call via `std::stringstream`

`FormatNumber()` is called every frame (HUD). `std::stringstream` allocates on the heap.
Use `snprintf` into a small stack buffer or raylib's `TextFormat` instead:

```cpp
inline const char* FormatNumber(float num) {
    if (num >= 1000000.0f) return TextFormat("%.2fM", num / 1000000.0f);
    if (num >= 1000.0f)    return TextFormat("%.2fK", num / 1000.0f);
    return TextFormat("%.0f", num);
}
```

(Note: `TextFormat` uses a static buffer — fine for immediate-mode drawing.)

### 21. `HudView::draw()` — repeated `GetScreenWidth()` / `GetScreenHeight()` calls

The function calls these multiple times per frame. Cache them at the top:

```cpp
const int screenW = GetScreenWidth();
const int screenH = GetScreenHeight();
```

### 22. `using json = nlohmann::json;` in headers

Declared in `GameScreen.h` and `Utils.h`. This forces the full `json.hpp` (~25K lines) into every
translation unit that includes these headers, slowing compilation. Move the alias and include into
`.cpp` files only.

### 23. `Vector3Add` chain in `GameData::applyForces()`

```cpp
const auto total = Vector3Add(Vector3Add(Vector3Add(thrustForce, dragForce), weightForce), liftForce);
```

This creates 2 temporary `Vector3` values. A helper or manual addition avoids them:

```cpp
const Vector3 total = {
    thrustForce.x + dragForce.x + weightForce.x + liftForce.x,
    thrustForce.y + dragForce.y + weightForce.y + liftForce.y,
    thrustForce.z + dragForce.z + weightForce.z + liftForce.z,
};
```

Minor at 60 fps, but good practice for inner-loop math.

---

## ✅ Best Practices

### 24. No RAII wrapper for raylib resources

Every screen manually calls `UnloadTexture`, `UnloadShader`, etc. in destructors. If a constructor
throws (or you forget an unload — as with `futuristicCity`), you leak GPU resources. Consider a
small RAII wrapper:

```cpp
struct TextureHandle {
    Texture2D tex;
    TextureHandle(const char* path) : tex(LoadTexture(path)) {}
    ~TextureHandle() { UnloadTexture(tex); }
    TextureHandle(const TextureHandle&) = delete;
    TextureHandle& operator=(const TextureHandle&) = delete;
    operator Texture2D() const { return tex; }
};
```

### 25. `SplashScreen` — fully implemented in the header

`SplashScreen.h` contains the full constructor, destructor, `update()`, and `run()` bodies.
This means every file that includes it recompiles when any implementation detail changes. Move
the bodies to a `.cpp` file.

### 26. Pass `const AppConfig&` where mutation isn't needed

`GameScreen`, `View`, and most derived classes take `AppConfig&` (non-const reference), but none
of them modify the config. Use `const AppConfig&` to express intent and prevent accidental
mutation.

### 27. Clamp throttle in one place

Throttle is clamped in `GameData::applyState()` to `[0, 1.2]`, but it's also directly set in
`GameplayScreen::handleInputs()` (where `KEY_A` sets it to `1.2f` and `KEY_ZERO` to `0.0f`).
The increment/decrement path (`KEY_MINUS`/`KEY_EQUAL`) doesn't clamp at all — you can hold `+`
and exceed `1.2` for one frame before `applyState` corrects it. Apply the clamp immediately after
any assignment.

### 28. `GameScreen` — missing copy/move protection

`GameScreen` has a virtual destructor, which is correct, but lacks deleted copy/move operations.
Screens hold GPU resources — accidental copies would double-free them.

```cpp
GameScreen(const GameScreen&) = delete;
GameScreen& operator=(const GameScreen&) = delete;
```

### 29. CMake — source files listed manually

Every new `.cpp`/`.h` requires editing `CMakeLists.txt`. For a small project this is fine, but
you could use `file(GLOB_RECURSE …)` during development (not recommended for production builds)
or at least group with comments for maintainability.

### 30. `HudView` uses `std::pmr::vector`

```cpp
std::pmr::vector<Color> colrs = {GREEN, WHITE, BLACK};
```

This uses polymorphic memory resources — almost certainly unintentional. Use `std::vector<Color>`.

### 31. Consider `const` on methods

`GameplayScreen::handleSounds()` is correctly `const` — but `handleInputs()`, and several view
`draw()` methods could also be `const` if they don't modify state. This helps the compiler and
documents intent.

---

## 📋 Summary

| Category | Count |
|----------|-------|
| Bugs | 8 |
| Architecture | 5 |
| Readability | 4 |
| Performance | 6 |
| Best practices | 8 |

**Overall impression:** The project has a clean, well-organized structure — the screen state
machine, the view system, and the physics/autopilot separation show good architectural thinking.
The main areas to improve are: fix the handful of real bugs (especially the shader and the
hardcoded ground-altitude values), tighten up naming consistency, and consider splitting `GameData`
before it grows further. The physics code itself is solid and well-commented.

Good luck with the project — looking forward to flying that F-15! ✈️
