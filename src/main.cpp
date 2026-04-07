#include "raylib.h"
#include "rlgl.h"
#include <vector>
#include <map>
#include <memory>
#include "GameScreen.h"
#include "MainMenuScreen.h"
#include "SplashScreen.h"
#include "GameplayScreen.h"
#include "BriefingScreen.h"
#include "primitives/AppConfig.h"
#include "primitives/Logger.h"
#include "scenario/Scenario.h"

int main() {
    SetTraceLogCallback(CustomLogCallback);
    SetTraceLogLevel(LOG_DEBUG);
    SetConfigFlags(FLAG_VSYNC_HINT);

    // todo make a check before changing the directory to avoid the warning
    // ChangeDirectory(TextFormat("%s../Resources", GetApplicationDirectory()));
    TraceLog(LOG_DEBUG, "Working directory is: %s", GetWorkingDirectory());

    const auto config = std::make_unique<AppConfig>();

    InitAudioDevice();
    InitWindow(
        config->get<int>("/config/screenWidth"),
        config->get<int>("/config/screenHeight"),
        config->get<std::string_view>("/name").data()
    );
    SetExitKey(KEY_BACKSPACE);

    // SetTargetFPS(60);
    // set how far we can see in 3d mode
    rlSetClipPlanes(config->get<float>("/config/nearPlane"), config->get<float>("/config/farPlane"));

    Scenario activeScenario;

    // load first screen
    std::unique_ptr<GameScreen> currentScreen = std::make_unique<SplashScreen>(*config);
    auto currentState = ScreenState::SPLASH;


    while (!WindowShouldClose()) {
        // screens state machine
        if (const ScreenState nextState = currentScreen->update(); nextState != currentState) {
            switch (nextState) {
                case ScreenState::MAIN_MENU:
                    currentScreen = std::make_unique<MainMenuScreen>(*config, activeScenario);
                    break;
                case ScreenState::BRIEFING:
                    currentScreen = std::make_unique<BriefingScreen>(*config, activeScenario);
                    break;
                case ScreenState::GAMEPLAY:
                    currentScreen = std::make_unique<GameplayScreen>(*config, activeScenario);
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

    CloseWindow();
    CloseAudioDevice();
    return 0;
}
