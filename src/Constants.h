#pragma once
#include "raylib.h"
#include <string_view>

namespace GameConfig {
    // global
    inline constexpr int SCREEN_WIDTH = 1280;
    inline constexpr int SCREEN_HEIGHT = 720;
    // inline constexpr int VIRTUAL_WIDTH = 800;
    // inline constexpr int VIRTUAL_HEIGHT = 600;
    inline constexpr std::string_view GAME_TITLE = "AIRBORNE";

    // splash
    inline constexpr std::string_view SPLASH_PATH = "res/splash-3.png";
    inline constexpr std::string_view INTRO_MUSIC_PATH = "res/sound0.mp3";

    // game
    inline constexpr std::string_view COCKPIT_OVERLAY_PATH = "res/cockpit-05.png";

    // airplane behavior
    inline constexpr float MAX_SPEED = 4000.0f;
    inline constexpr float MIN_SPEED = 0.0f;
    inline constexpr float PITCH_RATIO = 1.0f;
    inline constexpr float ROLL_RATIO = 1.0f;
    inline constexpr float YAW_RATIO = 0.5f;
    inline constexpr float SPEED_RATIO = 10.0f;

    // auto leveling
    inline constexpr float AUTO_LEVEL_SPEED = 1.0f;

    // auto pilot
    inline constexpr float AUTO_PILOT_MAX_BANK_ANGLE = 60.f;
    inline constexpr float AUTO_PILOT_PULL_RATIO = 0.8f;
}

/**
 * Some definitions for our world
 */
namespace GamePhysics {
    constexpr Vector3 WorldForward = {0.0f, 0.0f, 1.0f};
    constexpr Vector3 WorldUp = {0.0f, 1.0f, 0.0f};
    constexpr Vector3 WorldRight = {-1.0f, 0.0f, 0.0f};
}
