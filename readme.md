# Airborne (temporary name)

This project trys to make a little bit modern remake of the
classic [F-15 Strike Eagle II](https://en.wikipedia.org/wiki/F-15_Strike_Eagle_II) and others flight simulators
made by [MicroProse](https://en.wikipedia.org/wiki/MicroProse) back in the 80s.

The magic of those games comes from the fact it is not really a simulator, and it is not really an arcade. It somthing
between just genius like [Sid Meier](https://en.wikipedia.org/wiki/Sid_Meier) can create :).

Check out the [development blog](https://ziv.github.io/airborne/) for updates.

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

## Credits

Only items currently in use in the compiled code:

- Models by [Uxxman](https://sketchfab.com/Uxxman)

## Keyboard Controls

### Flight

- `ARROW UP` - Pitch down
- `ARROW DOWN` - Pitch up
- `ARROW LEFT` - Roll left
- `ARROW RIGHT` - Roll right
- `Q` - Yaw left
- `E` - Yaw right
- `=` - Increase throttle
- `-` - Decrease throttle
- `0` - Throttle 0% (cut engines)
- `1`–`9` - Throttle presets (10% to 90%)
- `A` - Afterburner (120% throttle)
- `B` - Toggle brakes
- `G` - Toggle landing gear

### Game

- `P` - Pause / resume
- `T` - Toggle autopilot
- `SPACE` - Return to main menu (after crash)
- `BACKSPACE` - Exit game

### Navigation / Menus

- `ARROW UP` / `ARROW DOWN` - Navigate menu items
- `ENTER` / `SPACE` - Confirm selection
- `ESCAPE` - Back / cancel
- `/` - Open help screen

## Display Controls

- `F1` (hold) - External chase camera
- `F2` - Cycle left cockpit pane
- `F3` - Cycle center cockpit pane
- `F4` - Cycle right cockpit pane
- `R` - Toggle radar mode
- `Z` - Map zoom in
- `X` - Map zoom out
- `ALT` + `H` - Toggle HUD colors (green / white / black)
- `ALT` + `D` - Toggle debug overlay