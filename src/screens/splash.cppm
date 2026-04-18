module;

export module Screens:Splash;

import :Base;

export class SplashScreen : public BaseScreen {
    ScreenState update() override {
        return ScreenState::SPLASH;
    }

    void run() override {
    }
};
