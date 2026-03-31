#include "raylib.h"
#include "rlgl.h"
#include <vector>
#include <map>
#include <memory>
#include "GameScreen.h"
#include "MainMenuScreen.h"
#include "SplashScreen.h"
#include "GameplayScreen.h"
#include "primitives/AppConfig.h"

int main() {
    SetTraceLogLevel(LOG_INFO);
    SetConfigFlags(FLAG_VSYNC_HINT);

    const auto config = std::make_unique<AppConfig>();

    InitAudioDevice();
    InitWindow(
        config->screenWidth,
        config->screenHeight,
        config->name.c_str()
    );

    SetTargetFPS(60);
    // set how far we can see in 3d mode
    rlSetClipPlanes(1.0f, config->clipPlans);

    // load first screen
    std::unique_ptr<GameScreen> currentScreen = std::make_unique<SplashScreen>(*config);
    auto currentState = ScreenState::SPLASH;


    while (!WindowShouldClose()) {
        // screens state machine
        if (const ScreenState nextState = currentScreen->update(); nextState != currentState) {
            switch (nextState) {
                case ScreenState::MAIN_MENU:
                    currentScreen = std::make_unique<MainMenuScreen>(*config);
                    break;
                case ScreenState::GAMEPLAY:
                    currentScreen = std::make_unique<GameplayScreen>(*config);
                    break;
                default:
                    currentScreen = std::make_unique<SplashScreen>(*config);
                    break;
            }
            currentState = nextState;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        currentScreen->run();
        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}