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
        src/utils/loaders.h
        src/primitives/AppConfig.h
        src/primitives/AppConfig.cpp
        src/GameScreen.h
        src/MainMenuScreen.cpp
        src/MainMenuScreen.h
        src/SplashScreen.h
        src/Constants.h
        src/GameplayScreen.cpp
        src/GameplayScreen.h
        src/primitives/Autopilot.cpp
        src/primitives/Autopilot.h
        src/primitives/Utils.h
        src/primitives/GameData.cpp
        src/primitives/GameData.h
        src/items/Structure.cpp
        src/items/Structure.h
        src/items/Aircraft.cpp
        src/items/Aircraft.h
        src/views/HudView.cpp
        src/views/HudView.h
        src/views/DebugView.cpp
        src/views/DebugView.h
        src/primitives/View.h
        src/primitives/gui_impl.cpp
        src/views/GaugesView.cpp
        src/views/GaugesView.h
        src/views/MapView.cpp
        src/views/MapView.h
)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
