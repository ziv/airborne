# Code Review — Airborne (2nd Pass)

Good progress since the first review — many issues were fixed (deltaTime consistency,
default member values, static constexpr, cockpit path, dead code in main, speed clamping).
This review focuses on what remains and takes a deep dive into `DrawHud()`.

---

## 🔴 Bugs

### 1. Duplicate cubes at the same position
**File:** `GameplayScreen.cpp:53-54`
```cpp
DrawCube({-100.0f, 10.0f, -100.0f}, 10.0f, 10.0f, 10.0f, PURPLE);
DrawCube({-100.0f, 10.0f, -100.0f}, 10.0f, 10.0f, 10.0f, GREEN);
```
Two cubes at the exact same coordinates — GREEN completely overwrites PURPLE.
One of them should have a different position.

---

## 🟡 Code Quality

### 2. Unused includes in `main.cpp`
```cpp
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>
```
None of these are used. Leftovers from the old commented-out code (which is now gone — nice).

### 3. `ScreenState::EXIT` is never handled
The enum defines `EXIT`, but the switch in `main.cpp` falls through to `default` → `SplashScreen`.
A screen can never quit the game through the state machine.

### 4. `raygui.h` sits unused in `src/`
Not included anywhere, not in CMakeLists.txt.

### 5. Typo: "plan" → "plane" in GameCamera.h doc comments
Lines 39, 49, 54: *"Initial place of the plan"*, *"Moving the plan"*, *"Bringing the plan to"*.

### 6. Magic number in cockpit draw
```cpp
DrawTexture(cockpit, -8, 0, WHITE);
```
The `-8` is unexplained. A named constant (e.g. `COCKPIT_OFFSET_X`) would clarify intent.

---

## 🔵 Deep Dive — `DrawHud()` pitch ladder

The author's own TODO says: *"rethink this calculation... it should be simpler and right
now it not working well."* Here's why, and a proposed fix.

### The problem: two conflicting coordinate systems

The function does two things using two different methods:

| What | How | Correct? |
|------|-----|----------|
| **Rung center positions** | 3D projection via `GetWorldToScreenEx` | ✅ Yes — uses the actual tilted camera |
| **Rung line direction** (`dx`,`dy`) | Dot-products against aircraft `right`/`up` | ❌ Approximate — ignores the camera tilt |

The camera is tilted down by `TiltDown` (0.45 rad ≈ 26°) relative to the aircraft nose.
The `dx,dy` direction is computed against the **aircraft's** up/right vectors, but the screen
is rendered through the **tilted camera**. During banked flight, this mismatch causes the
pitch ladder rungs to be visibly rotated relative to where they should be.

### Why it manifests during rolls

When the aircraft is level, `horizonDir3D` (the horizontal right vector) projects entirely
onto the camera's horizontal axis — the tilt doesn't affect it. But when the aircraft rolls,
the horizon direction has components in both `right` and `up`. The `up` component differs
between the aircraft frame and the camera frame by the tilt angle, causing the rung lines
to be off by several degrees.

### Also: `flatRight` and `horizonDir3D` are the same vector

Both compute the horizontal right direction from the aircraft's forward vector:
```cpp
flatRight   = normalize(cross(flatForward, WorldUp))   // line 101
horizonDir3D = normalize(cross(forward, WorldUp))       // line 103
```
Since `flatForward = normalize({forward.x, 0, forward.z})`, after normalization both
give the same direction: `{-forward.z, 0, forward.x}` (normalized). The only difference
is their edge-case fallbacks (`up` projection vs `right`), making this a confusing
near-duplication.

### The fix: derive line direction from projection too

Instead of computing `dx,dy` from dot products, project **two points per rung** through
the camera and use the screen-space vector between them. This eliminates the coordinate
frame mismatch entirely.

### Proposed rewrite

```cpp
void GameplayScreen::DrawHud() const {
    constexpr int hudSize = 280;
    constexpr int hudX = (GameConfig::SCREEN_WIDTH - hudSize) / 2;
    constexpr int hudY = (GameConfig::SCREEN_HEIGHT - hudSize) / 2 - 100;

    BeginScissorMode(hudX, hudY, hudSize, hudSize);

    const Camera rayCam = playerCamera.GetRaylibCamera();
    const Vector3 camForward = Vector3Normalize(
        Vector3Subtract(rayCam.target, rayCam.position));
    const Vector3 forward = playerCamera.GetForward();
    const Vector3 up      = playerCamera.GetUp();

    // --- heading projected to horizontal plane ---
    Vector3 flatForward = {forward.x, 0.0f, forward.z};
    if (Vector3Length(flatForward) < 0.001f)
        flatForward = {up.x, 0.0f, up.z};
    flatForward = Vector3Normalize(flatForward);

    const Vector3 flatRight = Vector3Normalize(
        Vector3CrossProduct(flatForward, GamePhysics::WorldUp));

    // --- screen-space sky reference (computed once, valid for all rungs) ---
    const Vector3 refPt  = Vector3Add(rayCam.position,
                                      Vector3Scale(camForward, 10000.0f));
    const Vector3 skyPt  = Vector3Add(refPt,
                                      Vector3Scale(GamePhysics::WorldUp, 500.0f));
    const Vector2 refScr = GetWorldToScreenEx(
        refPt, rayCam, GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT);
    const Vector2 skyScr = GetWorldToScreenEx(
        skyPt, rayCam, GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT);

    Vector2 skyRef = {skyScr.x - refScr.x, skyScr.y - refScr.y};
    const float skyRefLen = sqrtf(skyRef.x * skyRef.x + skyRef.y * skyRef.y);
    if (skyRefLen > 0.001f) {
        skyRef.x /= skyRefLen;
        skyRef.y /= skyRefLen;
    }

    // --- speed & altitude labels (relative to HUD rect) ---
    DrawText(TextFormat("%0.f", playerInput.Speed),
             hudX + 10, hudY + hudSize / 2, 15, GREEN);
    DrawText(TextFormat("%0.f", rayCam.position.y),
             hudX + hudSize - 30, hudY + hudSize / 2, 15, GREEN);

    // --- pitch ladder ---
    for (int angle = -80; angle <= 80; angle += 20) {
        const float pitchRad = static_cast<float>(angle) * PI / 180.0f;
        const Vector3 rungDir = Vector3RotateByAxisAngle(
            flatForward, flatRight, pitchRad);

        // cull if behind camera
        if (Vector3DotProduct(rungDir, camForward) < 0.1f) continue;

        // project rung center AND one offset point to get screen-space direction
        const Vector3 center3D = Vector3Add(
            rayCam.position, Vector3Scale(rungDir, 10000.0f));
        const Vector3 side3D   = Vector3Add(
            center3D, Vector3Scale(flatRight, 500.0f));

        const Vector2 center = GetWorldToScreenEx(
            center3D, rayCam, GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT);
        const Vector2 side   = GetWorldToScreenEx(
            side3D, rayCam, GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT);

        // screen-space rung direction
        Vector2 rd = {side.x - center.x, side.y - center.y};
        const float rdLen = sqrtf(rd.x * rd.x + rd.y * rd.y);
        if (rdLen < 0.001f) continue;
        rd.x /= rdLen;
        rd.y /= rdLen;

        // perpendicular — orient toward sky using the pre-computed reference
        Vector2 perp = {-rd.y, rd.x};
        if (perp.x * skyRef.x + perp.y * skyRef.y < 0.0f) {
            perp.x = -perp.x;
            perp.y = -perp.y;
        }
        // `perp` now points toward sky on screen

        const float halfLen  = (angle == 0) ? 110.0f : 90.0f;
        const float gapHalf  = 30.0f;
        const float thick    = (angle == 0) ? 2.0f : 1.0f;
        constexpr float tick = 15.0f;

        const Vector2 start = {center.x - rd.x * halfLen,
                               center.y - rd.y * halfLen};
        const Vector2 end   = {center.x + rd.x * halfLen,
                               center.y + rd.y * halfLen};
        const Vector2 gapL  = {center.x - rd.x * gapHalf,
                               center.y - rd.y * gapHalf};
        const Vector2 gapR  = {center.x + rd.x * gapHalf,
                               center.y + rd.y * gapHalf};

        // the two halves of the rung (gap in the middle)
        DrawLineEx(start, gapL, thick, GREEN);
        DrawLineEx(gapR, end,   thick, GREEN);

        if (angle == 0) {
            // horizon line — ticks point down (toward ground)
            DrawLineEx(start, {start.x - perp.x * tick,
                               start.y - perp.y * tick}, thick, GREEN);
            DrawLineEx(end,   {end.x   - perp.x * tick,
                               end.y   - perp.y * tick}, thick, GREEN);
        } else {
            // ticks always point toward the horizon (0°)
            const float sign = (angle > 0) ? -1.0f : 1.0f;
            DrawLineEx(start, {start.x + perp.x * sign * tick,
                               start.y + perp.y * sign * tick}, thick, GREEN);
            DrawLineEx(end,   {end.x   + perp.x * sign * tick,
                               end.y   + perp.y * sign * tick}, thick, GREEN);

            DrawText(TextFormat("%d", angle),
                     static_cast<int>(start.x) - 20,
                     static_cast<int>(start.y) - 5, 10, GREEN);
        }
    }
    EndScissorMode();
}
```

### What changed and why

| Change | Why |
|--------|-----|
| Removed `horizonDir3D`, `dx`, `dy`, `skyDirX`, `skyDirY` | Redundant — replaced by projected screen-space vectors |
| Added `side3D` projection per rung | Gives correct screen-space rung direction through the tilted camera |
| Added `skyRef` via projection | Robust sky-direction reference that works at any roll/pitch |
| `perp` oriented against `skyRef` | Always points toward sky regardless of camera orientation |
| Speed/altitude labels use `hudX`/`hudY` offsets | Labels follow the HUD if its position changes |

The total cost is **2 extra projections per visible rung** (≤9 rungs) + **2 for the sky
reference** — negligible on any hardware running raylib.

---

## 🟢 Minor Suggestions

### 7. Speed has no upper bound in the config
Speed is clamped to `[0, 2000]` inline in Update(). Consider adding
`MAX_SPEED` to `GameConfig` so it's easy to find and tweak.

### 8. `GameCamera::placeCamera` recomputes `GetForward/GetRight/GetUp`
`placeCamera()` calls `GetRight()`, `GetForward()`, and `GetUp()` which each
do `Vector3RotateByQuaternion`. Since `move()` already computed `localUp` and
`localRight` just before calling `placeCamera()`, these could be passed in to
avoid redundant quaternion rotations. Minor perf, but cleaner data flow.

---

## Summary

| Severity | Count |
|----------|-------|
| 🔴 Bug | 1 |
| 🟡 Code quality | 5 |
| 🔵 HUD rewrite | 1 (with full solution) |
| 🟢 Suggestion | 2 |

**Priority order:**
1. Replace the HUD pitch-ladder calculation (#7 — the biggest impact on gameplay feel)
2. Fix the duplicate cube (#1)
3. Clean up unused includes and handle `EXIT` state (#2, #3)
