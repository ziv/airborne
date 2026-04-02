cmake_minimum_required(VERSION 3.20)
project(se LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch raylib from GitHub
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib)

add_executable(${PROJECT_NAME} src/main.cpp
        src/lib/json.hpp
        src/core/AppConfig.h
        src/core/AppConfig.cpp
        src/GameScreen.h
        src/MainMenuScreen.cpp
        src/MainMenuScreen.h
        src/SplashScreen.h
        src/primitives/Constants.h
        src/GameplayScreen.cpp
        src/GameplayScreen.h
        src/core/Autopilot.cpp
        src/core/Autopilot.h
        src/primitives/Utils.h
        src/core/GameData.cpp
        src/core/GameData.h
        src/items/Structure.cpp
        src/items/Structure.h
        src/items/Aircraft.cpp
        src/items/Aircraft.h
        src/lib/gui_impl.cpp
        src/views/CockpitView.cpp
        src/views/CockpitView.h
        src/views/DebugView.cpp
        src/views/DebugView.h
        src/primitives/Types.h
        src/core/AircraftTransformation.cpp
        src/core/AircraftTransformation.h
        src/core/AircraftPhysics.cpp
        src/core/AircraftPhysics.h
        src/core/AircraftControls.cpp
        src/core/AircraftControls.h
        src/core/AircraftCamera.cpp
        src/core/AircraftCamera.h
        src/primitives/Resource.h
        src/views/MinihudView.cpp
        src/views/MinihudView.h
        src/views/WorldView.cpp
        src/views/WorldView.h
        src/views/NavballView.cpp
        src/views/NavballView.h
        src/primitives/Logger.h
        src/SplashScreen.cpp
        src/core/AircraftStructs.h
)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
