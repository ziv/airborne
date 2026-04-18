module;
#include "../lib/ray.hpp"
export module Screens:Splash;

import RaylibResource;
import JsonConfig;
import :Base;

export struct SplashScreenConfig {
  std::string bg_tex_path = "res/images/splash.png";
  std::string bg_sound_path = "res/audio/splash-1.mp3";
};

export {
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(SplashScreenConfig,
                                                  bg_tex_path, bg_sound_path);
}

export class SplashScreen : public BaseScreen {
  SplashScreenConfig conf;
  TextureHandle tex;
  MusicHandle music;

public:
  explicit SplashScreen()
      : conf(JsonConfig("assets/config.jsonc").get<SplashScreenConfig>("/screens/splash")),
        tex(LoadTexture(conf.bg_tex_path.c_str())),
        music(LoadMusicStream(conf.bg_sound_path.c_str())) {
    PlayMusicStream(music);
  }

  ScreenState update() override {
    UpdateMusicStream(music);
    return IsKeyPressed(KEY_SPACE) ? ScreenState::GAMEPLAY
                                   : ScreenState::SPLASH;
  }

  void draw() override {
    DrawTexture(tex, 0, 0, WHITE);
    DrawText("Created by Ziv Perry", 910, 640, 20, GOLD);
  }
};
