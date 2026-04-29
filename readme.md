# Airborne (temporary name)

An indie game project. Simple flight simulator.

This project trys to make a little bit modern remake of the
classic [F-15 Strike Eagle II](https://en.wikipedia.org/wiki/F-15_Strike_Eagle_II) and others flight simulators
made by [MicroProse](https://en.wikipedia.org/wiki/MicroProse) back in the 80s.

The magic of those games comes from the fact it is not really a simulator, and it is not really an arcade. It somthing
between just genius like [Sid Meier](https://en.wikipedia.org/wiki/Sid_Meier) can create :).

Check out the [development blog](https://ziv.github.io/airborne/) for updates ot
the [Wiki](https://github.com/ziv/airborne/wiki) for more information.

## How to

Still in progress, but...

```shell
cmake --build cmake-build-debug
./cmake-build-debug/se
```

## The Rules

- Do not use game engine and or LLM
- I have to re-learn CPP and linear algebra (ohh, trigonometry)
- To have fun

## Nostalgia Moment

I spent so many hours flying this beast on my XT-16MGhz machine with monochrome CGA resolution screen. I tried to play
it again, it was fun but somthing is missing. No, I don't want another DCS neither an arcade game, I want the same
gameplay I
got from this magic game, so this project has begun.

![ia](assets/images/f15-se-ii.png)

## TODOs

Leftovers from the ESC refactoring:

- [x] complete the terrain streaming
- [ ] move fog shader into terrain streamer
- [ ] complete configuring the terrain streamer 
- [ ] performance optimizations: change read only view data to const reference
- [x] ~~move the scene into a view~~
- [x] hud colors
- [x] bring the radar widget
- [x] complete radar items
- [x] bring the clouds effect into scene view
- [x] bring the sound into scene view
- [ ] complete the debug view by type
- [ ] bring the autopilot controller
- [x] check safe landing mechanism
- [x] bring scene sounds
- [ ] bring the navball widget
- [x] complete screens
- [x] complete crash layout
- [ ] refactor scene data into a scene config
- [ ] add braking on landing

## Modules Rules

```c++
module;
// includes only here:
#include <vector>

export module MyModule;

// import only here:
import JsonConfig;

export class MyClass { ... };
```

## Modern CPP Rules

- Modules
- `static_cast` for casting
- `if-init` statements
- `auto` for type inference
- `constexpr` for compile-time constants
- `nullptr` instead of `NULL`

Style Rules

- Use `PascalCase` for class names, structs, and enums
- Use `snake_case` for variables, methods, and functions

## Build

The project require C++ compiler with support for C++20 modules, and CMake 3.25 or higher.

I'm using LLVM. My configuration looks like this (macos):

```shell
export CC=$(brew --prefix llvm)/bin/clang
export CXX=$(brew --prefix llvm)/bin/clang++

cmake -B cmake-build-debug -S . -DCMAKE_BUILD_TYPE=Debug -G Ninja
cmake -B cmake-build-release -S . -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake -S . -B cmake-build-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address -g -fno-omit-frame-pointer" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"

# clean builds
cmake --build cmake-build-debug --target clean && cmake --build cmake-build-debug
```

New tiles map, 16m/pixel (world under assets)
Tile 2048x2048 pixels
Tile hm map 512x512 pixels

flight simulator
cpp with raylib and entt
ECS architecture
3d word
2d cockpit