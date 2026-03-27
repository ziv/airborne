#include "raylib.h"
#include <vector>
#include <map>
#include <iostream>
#include <memory>
#include <algorithm>
#include "Constants.h"
#include "GameScreen.h"
#include "MainMenuScreen.h"
#include "SplashScreen.h"
#include "GameplayScreen.h"

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT, GameConfig::GAME_TITLE.data());
    InitAudioDevice();
    SetTargetFPS(60);

    std::unique_ptr<GameScreen> currentScreen = std::make_unique<SplashScreen>();
    auto currentState = ScreenState::SPLASH;

    while (!WindowShouldClose()) {
        // screens state machine
        if (const ScreenState nextState = currentScreen->Update(); nextState != currentState) {
            switch (nextState) {
                case ScreenState::MAIN_MENU:
                    currentScreen = std::make_unique<MainMenuScreen>();
                    break;
                case ScreenState::GAMEPLAY:
                    currentScreen = std::make_unique<GameplayScreen>();
                    break;
                default:
                case ScreenState::SPLASH:
                    currentScreen = std::make_unique<SplashScreen>();
                    break;
            }
            currentState = nextState;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        currentScreen->Draw();
        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}