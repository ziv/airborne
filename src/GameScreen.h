#pragma once

enum class ScreenState {
    SPLASH,
    MAIN_MENU,
    GAMEPLAY,
    EXIT
};

class GameScreen {
public:
    virtual ~GameScreen() = default;

    virtual ScreenState Update() = 0;

    virtual void Draw() = 0;
};
