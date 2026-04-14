cmake_minimum_required(VERSION 3.20)
project(se LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Fetch raylib from GitHub
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib)

include(FetchContent)
FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.16.0
)
FetchContent_MakeAvailable(entt)

add_executable(${PROJECT_NAME} src/main.cpp
        src/lib/json.hpp
        src/primitives/AppConfig.h
        src/primitives/AppConfig.cpp
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
        src/lib/gui_impl.cpp
        src/views/CockpitView.cpp
        src/views/CockpitView.h
        src/views/DebugView.cpp
        src/views/DebugView.h
        src/views/cockpit/MapView.cpp
        src/views/cockpit/MapView.h
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
        src/views/cockpit/MinihudView.cpp
        src/views/cockpit/MinihudView.h
        src/views/NavballView.cpp
        src/views/NavballView.h
        src/primitives/Logger.h
        src/SplashScreen.cpp
        src/core/AircraftStructs.h
        src/core/SceneManager.cpp
        src/core/SceneManager.h
        src/views/Aircraft.cpp
        src/views/Aircraft.h
        src/views/cockpit/RadarView.cpp
        src/views/cockpit/RadarView.h
        # scenario system
        src/entities/Entity.h
        src/entities/EntityRegistry.h
        src/entities/EntityRegistry.cpp
        src/entities/GroundTarget.h
        src/entities/GroundTarget.cpp
        src/scenario/Objective.h
        src/scenario/Trigger.h
        src/scenario/Scenario.h
        src/scenario/ScenarioLoader.h
        src/scenario/ScenarioLoader.cpp
        src/BriefingScreen.h
        src/BriefingScreen.cpp
        src/HelpScreen.cpp
        src/HelpScreen.h
        src/views/cockpit/OffView.cpp
        src/views/cockpit/OffView.h
        src/views/cockpit/EngineView.cpp
        src/views/cockpit/EngineView.h
        src/primitives/Session.cpp
        src/primitives/Session.h
        src/lib/json.h
        src/scenario/Payload.h
        src/entities/Carrier.cpp
        src/entities/Carrier.h
        src/entities/Airbase.cpp
        src/entities/Airbase.h
        src/entities/Structure.cpp
        src/entities/Structure.h
        src/entities/Aaa.cpp
        src/entities/Aaa.h
        src/entities/Entity.cpp
        src/primitives/Math.h
        src/entities/EntitiesUtils.h
        src/components/world.h
        src/systems/physics.h
        src/lib/entt.h
        src/components/aircraft.h
        src/prefabs/player_creator.h
)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_link_libraries(${PROJECT_NAME} PRIVATE EnTT::EnTT)

#target_compile_options(${PROJECT_NAME} PRIVATE
#        -Wall
#        -Wextra
#        -Wpedantic
#)