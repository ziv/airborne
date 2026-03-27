#pragma once
#include <string_view>
#include "GameScreen.h"
#include "primitives/GameCamera.h"

struct GameplayConfig {
    std::string_view name;
};

class GameplayScreen : public GameScreen {
    // the camera is the airplane
    GameCamera playerCamera;
    Orientation playerInput{};
    Texture2D cockpit;

    void DrawHud() const;
    void DrawLegend() const;
public:
    GameplayScreen();

    ~GameplayScreen() override;

    ScreenState Update() override;

    void Draw() override;
};
