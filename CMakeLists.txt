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
        src/json.hpp
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
        src/views/Hud.h
        src/primitives/GameData.cpp
        src/primitives/GameData.h
        src/items/Structure.cpp
        src/items/Structure.h
        src/primitives/AppConfig.h
        src/views/Floater.h
)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
