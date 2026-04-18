cmake_minimum_required(VERSION 4.2.2)
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
project(airborne LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch raylib from GitHub
include(FetchContent)
FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

# Fetch entt from GitHub
include(FetchContent)
FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.16.0
)
FetchContent_MakeAvailable(entt)

# Fetch nlohmann_json from GitHub
include(FetchContent)
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(${PROJECT_NAME} src/main.cpp)

target_sources(${PROJECT_NAME} PUBLIC
        FILE_SET CXX_MODULES FILES
        src/game.cppm
        src/player/player.cppm
        src/player/player-camera.cppm
        src/player/player-config.cppm
        src/player/player-controls.cppm
        src/player/player-position.cppm
        src/player/player-physics.cppm
        src/player/player-rotation.cppm
        src/player/player-dispatcher.cppm
        src/player/ground-check.cppm
        src/core/types/types.cppm
        src/core/types/scenario-types.cppm
        src/core/types/units-types.cppm
        src/core/helpers.cppm
        src/core/json-config.cppm
        src/core/types/config-types.cppm
        src/core/raylib-resource.cppm
        src/core/resource-manager.cppm
        src/core/resource-preloader.cppm
        src/components/components.cppm
        src/components/world.cppm
        src/components/data.cppm
        src/components/render.cppm
        src/prefabs/prefabs.cppm
        src/prefabs/create-player.cppm
        src/prefabs/create-scene.cppm
        src/prefabs/create-unit.cppm
        src/prefabs/create-cockpit-widgets.cppm
        src/prefabs/create-cockpit.cppm
        src/prefabs/create-hud.cppm
        src/systems/renderers/render.cppm
        src/systems/renderers/render-models.cppm
        src/systems/renderers/render-cockpit.cppm
        src/systems/renderers/render-minimap.cppm
        src/systems/renderers/render-debug.cppm
        src/systems/renderers/render-debugging.cppm
        src/systems/renderers/render-engines-status.cppm
        src/systems/renderers/render-hud.cppm
        src/systems/renderers/render-radar.cppm
        src/systems/renderers/render-crash-layout.cppm
        src/systems/world-streamer.cppm
        src/systems/widgets-inputs.cppm
        src/utils/accessors.cppm
        src/utils/loading.cppm
        src/screens/base.cppm
        src/screens/screens.cppm
        src/screens/splash.cppm
        src/screens/game.cppm
)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib EnTT::EnTT nlohmann_json::nlohmann_json)
