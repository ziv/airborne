/**
 * @file main.cpp
 * @brief Application entry point — initialises raylib, loads config, and runs
 *        the screen state machine (SPLASH → MAIN_MENU → BRIEFING → GAMEPLAY).
 */
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
#include "HelpScreen.h"
#include "primitives/AppConfig.h"
#include "primitives/Logger.h"
#include "scenario/Scenario.h"

int main() {
    SetTraceLogCallback(CustomLogCallback);
    SetTraceLogLevel(LOG_DEBUG);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);


    // todo make a check before changing the directory to avoid the warning
    // ChangeDirectory(TextFormat("%s../Resources", GetApplicationDirectory()));
    TraceLog(LOG_DEBUG, "Working directory is: %s", GetWorkingDirectory());

    const auto config = std::make_unique<AppConfig>();
    const auto width = config->get<int>("/config/screenWidth");
    const auto height = config->get<int>("/config/screenHeight");

    InitAudioDevice();
    InitWindow(width, height, config->get<std::string_view>("/name").data());
    SetExitKey(KEY_BACKSPACE);
    // todo uncomment to support fullscreen
    // const int monitor = GetCurrentMonitor();
    // SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
    // ToggleBorderlessWindowed();
    // const RenderTexture2D target = LoadRenderTexture(width, height);
    // SetTextureFilter(target.texture, TEXTURE_FILTER_POINT); TEXTURE_FILTER_BILINEAR


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
                case ScreenState::HELP:
                    currentScreen = std::make_unique<HelpScreen>(*config, activeScenario);
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


        // todo uncomment to support fullscreen
        // BeginTextureMode(target);
        // ClearBackground(BLACK);
        // currentScreen->run();
        // EndTextureMode();
        //
        // BeginDrawing();
        // float scale = fmin((static_cast<float>((GetScreenWidth()) / width, (float) GetScreenHeight() / height);
        // DrawTexturePro(target.texture,
        //                {0.0f, 0.0f, (float) target.texture.width, (float) -target.texture.height}, // המקור (ה-y הפוך כי OpenGL הופך טקסטורות)
        //                {
        //                    (GetScreenWidth() - ((float) width * scale)) * 0.5f, (GetScreenHeight() - ((float) height * scale)) * 0.5f,
        //                    // המיקום הממורכז
        //                    (float) width * scale, (float) height * scale
        //                }, // הגודל המתוח
        //                {0, 0}, 0.0f, WHITE
        // );
        // EndDrawing();
    }

    CloseWindow();
    CloseAudioDevice();
    return 0;
}
