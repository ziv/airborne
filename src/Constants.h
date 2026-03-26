#pragma once
#include <string_view>

namespace GameConfig {
    inline constexpr int SCREEN_WIDTH = 1280;
    inline constexpr int SCREEN_HEIGHT = 720;
    inline constexpr int VIRTUAL_WIDTH = 800;
    inline constexpr int VIRTUAL_HEIGHT = 600;

    inline constexpr std::string_view SPLASH_PATH = "res/seorg.png";
    inline constexpr std::string_view INTRO_MUSIC_PATH = "res/sound1.mp3";

    inline constexpr std::string_view GAME_TITLE = "AIRBORNE";
}