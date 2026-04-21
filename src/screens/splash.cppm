module;
#include "../lib/ray.hpp"
export module Screens:Splash;

import RaylibResource;
import JsonConfig;
import Types;
import Accessors;
import :Base;

export class SplashScreen : public BaseScreen {
  TextureHandle tex;
  MusicHandle music;

 public:
  explicit SplashScreen(const SplashScreenConfig& config) : tex(LoadTexture(config.bg_tex_path.c_str())), music(LoadMusicStream(config.bg_sound_path.c_str())) {
    PlayMusicStream(music);
  }

  ScreenState update() override {
    UpdateMusicStream(music);
    return IsKeyPressed(KEY_SPACE) ? ScreenState::LOADING : ScreenState::SPLASH;
  }

  void draw() override {
    DrawTexture(tex, 0, 0, WHITE);
    DrawText("Created by Ziv Perry", 910, 640, 20, GOLD);
  }
};
