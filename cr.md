# Code Review — Airborne

Overall: Nice clean architecture for an early-stage project. The screen state-machine pattern is solid,
the quaternion-based flight model is well thought out, and the HUD pitch-ladder is impressive work.
Below are the issues I found, ordered by severity.

---

## 🔴 Bugs / Will Not Compile

### 1. `playerCamera.worldUp` does not exist
**File:** `GameplayScreen.cpp:82`
```cpp
Vector3 horizonDir3D = Vector3CrossProduct(forward, playerCamera.worldUp);
```
`GameCamera` has no `worldUp` member. This will fail to compile.
**Fix:** Either add `const Vector3 worldUp = GamePhysics::WorldUp;` to `GameCamera`, or use `GamePhysics::WorldUp` directly here.

### 2. Speed clamped under wrong variable name
**File:** `GameplayScreen.cpp:34`
```cpp
if (playerInput.Pitch > 2000.0f) playerInput.Pitch = 2000.0f;
```
Pitch values are tiny (±1.0 × deltaTime ≈ ±0.016). A cap of 2000 is meaningless for pitch.
This was almost certainly meant to be:
```cpp
if (playerInput.Speed > 2000.0f) playerInput.Speed = 2000.0f;
```

### 3. `Constants.h` uses `Vector3` without including its header
**File:** `Constants.h:24-26`
```cpp
namespace GamePhysics {
    constexpr Vector3 WorldForward = {0.0f, 0.0f, 1.0f};
```
`Vector3` comes from `raylib.h`/`raymath.h`, but `Constants.h` only includes `<string_view>`.
This compiles by accident when every `.cpp` that includes `Constants.h` also happens to include
`raylib.h` first. It will break the moment someone includes `Constants.h` before `raylib.h`.
**Fix:** Add `#include "raylib.h"` to `Constants.h`.

---

## 🟡 Code Quality Issues

### 4. ~475 out of 528 lines in `main.cpp` are commented-out dead code
The active code is only ~50 lines. The remaining 90% is legacy code from a previous iteration
(terrain chunking, old camera code, Hebrew comments, etc.).
**Fix:** Delete it. It lives in git history if you ever need it.

### 5. Unused includes in `main.cpp`
```cpp
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>
```
None of these are used by the active code. They are leftovers from the commented-out block.

### 6. Double semicolons
- `GameplayScreen.cpp:22` — `playerInput.Pitch = 1.0f * deltaTime;;`
- `main.cpp:31` — `break;;`

Harmless but sloppy.

### 7. Inconsistent cockpit path — constant declared but unused
`Constants.h` defines:
```cpp
inline constexpr std::string_view COCKPIT_OVERLAY_PATH = "res/cockpit-1.png";
```
But `GameplayScreen.cpp:5` loads a different file with a hardcoded string:
```cpp
cockpit(LoadTexture("res/cockpit-05.png"))
```
The constant is never used, and the paths don't even match.

### 8. `GameState` class is completely empty
`GameState.h` and `GameState.cpp` declare/define an empty class with no members or methods.
Either remove it or flesh it out. Dead scaffolding adds confusion.

### 9. `GameplayConfig` struct declared but never used
**File:** `GameplayScreen.h:6-8`
```cpp
struct GameplayConfig {
    std::string_view name;
};
```
Not referenced anywhere.

### 10. `raygui.h` in `src/` — unused
The file exists in the source directory but is not included by any source file and not listed
in `CMakeLists.txt`. A ~22 KB third-party header sitting unused in `src/`.

### 11. `ScreenState::EXIT` is never handled
The enum defines `EXIT`, but the switch in `main.cpp` has no case for it — it falls through
to `default`, which creates a `SplashScreen`. If a screen ever returns `EXIT`, the game won't quit.

---

## 🟢 Design Suggestions

### 12. Inconsistent deltaTime handling in `GameCamera::move()`
`GameplayScreen::Update()` multiplies pitch/yaw/roll by `deltaTime` before passing them via
`Orientation`. But inside `GameCamera::move()`, `GetFrameTime()` is called *again* for
`bankInducedYaw` and `liftLossPitch`. This means:
- Player inputs use the caller's `deltaTime`
- Physics effects query their own `GetFrameTime()`

These should be the same value. Pass `deltaTime` explicitly through `Orientation` or as a parameter.

### 13. `Orientation` members have no default values
```cpp
struct Orientation {
    float Pitch;
    float Yaw;
    float Roll;
    float Speed;
};
```
While `GameplayScreen` uses `playerInput{}` (zero-initializes), a bare `Orientation o;` would
leave all fields uninitialized. Add defaults:
```cpp
float Pitch = 0.0f;
float Yaw = 0.0f;
float Roll = 0.0f;
float Speed = 0.0f;
```

### 14. `TiltDown` and `FieldOfView` should be `static constexpr`
**File:** `GameCamera.h:20-21`
```cpp
const float TiltDown = 0.45f;
const float FieldOfView = 85.0f;
```
These don't vary per-instance. Making them `static constexpr` expresses intent and avoids
per-object storage.

### 15. Magic number in cockpit draw
**File:** `GameplayScreen.cpp:58`
```cpp
DrawTexture(cockpit, -8, 0, WHITE);
```
The `-8` x-offset is unexplained. A named constant would help.

### 16. No speed upper bound
Speed has a lower clamp at 0 but can grow without limit. For a flight sim, terminal velocity
or engine max-thrust would be a natural cap.

---

## Summary

| Severity | Count |
|----------|-------|
| 🔴 Bug / Compile error | 3 |
| 🟡 Code quality | 8 |
| 🟢 Design suggestion | 5 |

The architecture (screen state machine, quaternion camera, HUD ladder) is solid groundwork.
The main priorities before moving forward should be:
1. Fix the three compile/logic bugs (#1, #2, #3)
2. Clean up the massive commented-out block in `main.cpp`
3. Align the cockpit constants so the config is actually used
