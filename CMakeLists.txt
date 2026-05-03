cmake_minimum_required(VERSION 4.2.2)
#set(CMAKE_CXX_COMPILER_LAUNCHER ccache)

project(airborne LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

include(FetchContent)
FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.16.0
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(entt)

include(FetchContent)
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
        httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.18.5
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(
        raygui
        GIT_REPOSITORY https://github.com/raysan5/raygui.git
        GIT_TAG 4.0
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raygui)

# OpenSSL (system-provided — on macOS: brew install openssl)
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp src/lib/raygui-impl.cpp src/lib/httplib-impl.cpp)

target_sources(${PROJECT_NAME} PUBLIC
        FILE_SET CXX_MODULES FILES
        src/game.cppm
        src/core/types/types.cppm
        src/core/types/scenario-types.cppm
        src/core/types/units-types.cppm
        src/core/types/config-types.cppm
        src/core/resources/resources.cppm
        src/core/resources/raylib-resource.cppm
        src/core/resources/resource-manager.cppm
        src/core/tiles/terrain-streamer.cppm
        src/core/tiles/terrain-utils.cppm
        src/core/tiles/map-streamer.cppm
        src/core/tiles/tile-downloader.cppm
        src/components/components.cppm
        src/components/widgets-components.cppm
        src/components/player-components.cppm
        src/components/global-components.cppm
        src/prefabs/prefabs.cppm
        src/prefabs/create-player.cppm
        src/prefabs/create-scene.cppm
        src/prefabs/create-unit.cppm
        src/prefabs/create-cockpit-widgets.cppm
        src/prefabs/create-cockpit.cppm
        src/prefabs/create-hud.cppm
        src/prefabs/spawn-entities.cppm
        src/systems/npc/npc.cppm
        src/systems/npc/npc-setup.cppm
        src/systems/npc/npc-steering.cppm
        src/systems/npc/npc-physics.cppm
        src/systems/npc/npc-autopilot.cppm
        src/systems/player/player.cppm
        src/systems/player/player-controls.cppm
        src/systems/player/player-camera.cppm
        src/systems/player/player-ground-check.cppm
        src/systems/player/player-physics.cppm
        src/systems/player/player-position.cppm
        src/systems/player/player-rotation.cppm
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
        src/systems/renderers/render-target-camera.cppm
        src/systems/game-options.cppm
        src/systems/aircraft/widgets-inputs.cppm
        src/systems/aircraft/aircraft.cppm
        src/systems/aircraft/engine-system.cppm
        src/systems/aircraft/gear-system.cppm
        src/systems/aircraft/update-lock-system.cppm
        src/utils/accessors.cppm
        src/utils/loading.cppm
        src/utils/utils.cppm
        src/utils/helpers.cppm
        src/utils/json-config.cppm
        src/screens/base.cppm
        src/screens/screens.cppm
        src/screens/splash.cppm
        src/screens/game.cppm
)


target_link_libraries(${PROJECT_NAME} PRIVATE raylib EnTT::EnTT nlohmann_json::nlohmann_json OpenSSL::SSL OpenSSL::Crypto)

if(APPLE)
    find_library(SECURITY_FRAMEWORK Security)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SECURITY_FRAMEWORK})
endif()


target_include_directories(${PROJECT_NAME} PRIVATE ${httplib_SOURCE_DIR})

target_include_directories(airborne PRIVATE ${raygui_SOURCE_DIR}/src)

# httplib needs its include dir and the OPENSSL support flag
target_compile_definitions(${PROJECT_NAME} PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
