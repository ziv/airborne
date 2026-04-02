# Airborne (temporary name)

This project trys to make a little bit modern remake of the
classic [F-15 Strike Eagle II](https://en.wikipedia.org/wiki/F-15_Strike_Eagle_II) and others flight simulators
made by [MicroProse](https://en.wikipedia.org/wiki/MicroProse) back in the 80s.

The magic of those games comes from the fact it is not really a simulator, and it is not really an arcade. It somthing
between just genius like [Sid Meier](https://en.wikipedia.org/wiki/Sid_Meier) can create :).

## How to

Still in progress, but...

```shell
cmake --build cmake-build-debug
./cmake-build-debug/se
```

## The Rules

- Do not use game engine
- I have to re-learn CPP and linear algebra (ohh, trigonometry)
- To have fun

## Nostalgia Moment

I spent so many hours flying this beast on my XT-16MGhz machine with monochrome CGA resolution screen. I tried to play
it again, it was fun but somthing is missing. No, I don't want another DCS neither an arcade game, I want the same
gameplay I
got from this magic game, so this project has begun.

![ia](res/f15-se-ii.png)

### Image processing

$64.82m/px = 281,985m^2$

$531,005.44m$ per axis.

# HUD impl as LLM understand

I let LLM to examine my solution...

## Current Implementation Analysis

The current `HudView::draw()` does the following for the pitch ladder:

1. Gets aircraft orientation: forward, up vectors from `GameData`
2. Computes flat heading: Projects forward vector onto horizontal plane (removes Y component) → `flatForward`
3. Computes flat right: Cross product of flatForward × WorldUp → `flatRight`
4. Sky reference (for knowing which way is "up" on screen):
    - Projects a far-away reference point and a point slightly above it to screen space
    - Computes the screen-space "up" direction (skyRef)
5. For each pitch rung (-80° to 80° in 20° steps):
    - Rotates `flatForward` around `flatRight` by the pitch angle to get the 3D direction to that rung
    - Projects this rung direction far out into 3D space to get its screen position
    - Also projects a side offset to determine the rung's screen-space orientation
    - Calculates the normalized screen-space direction of the rung line and rotates it 90° to get the perpendicular direction, flipping it to point toward the sky
    - Renders the rung lines and tick marks

Now I'm seeing the performance issues with this approach. The main bottleneck is calling the world-to-screen projection function repeatedly—20 times per frame just for the pitch
rungs. Projecting points that are 10,000 units away also introduces floating-point precision problems at the frustum edges. Plus there's redundant calculation happening with the
direction vectors.

The real insight is that a pitch ladder is just a 2D overlay. I don't need to project anything into screen space—I can compute the screen-space roll angle and pitch offset
directly from the aircraft's orientation angles without any 3D math at all. This is how real aircraft HUDs actually work.

To make this work, I need three things: the roll angle (which tells me how much the horizon tilts on screen), the pitch angle (which tells me where the horizon sits relative to
the aircraft's nose), and a pixels-per-degree scaling factor based on the camera's field of view and screen dimensions.