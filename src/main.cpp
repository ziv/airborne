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
        config->screenWidth(),
        config->screenHeight(),
        config->name().data()
    );

    SetTargetFPS(60);
    // set how far we can see in 3d mode
    rlSetClipPlanes(1.0f, config->clipPlans());

    // load first screen
    // std::unique_ptr<GameScreen> currentScreen = std::make_unique<SplashScreen>(*config);
    // auto currentState = ScreenState::SPLASH;
    std::unique_ptr<GameScreen> currentScreen = std::make_unique<GameplayScreen>(*config);
    auto currentState = ScreenState::GAMEPLAY;


    while (!WindowShouldClose()) {
        // screens state machine
        if (const ScreenState nextState = currentScreen->Update(); nextState != currentState) {
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

// Image textureImage = LoadImage("res/texture.png");
// const Texture2D texture = LoadTextureFromImage(textureImage);
// UnloadImage(textureImage);
//
// Image heightImage = LoadImage("res/heightmap.png");
// // Vector3 size = {10000.0f, 10000.0f, 10000.0f};
// Vector3 size = {2000.0f, 2000.0f, 2000.0f};
// Mesh mesh = GenMeshHeightmap(heightImage, size);
// UnloadImage(heightImage);
//
// Model model = LoadModelFromMesh(mesh);
// model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
