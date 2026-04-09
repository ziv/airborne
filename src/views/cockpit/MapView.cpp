/**
 * @file MapView.cpp
 * @brief Top-down tactical map rendering with Camera2D.
 */
#include "MapView.h"
#include "raylib.h"
#include "raymath.h"

MapView::MapView(const AppConfig &config) : tex(LoadTexture(config.get<std::string_view>("/views/mapViewTexture").data())) {
    // todo removed until better shader for glass
    // glass(LoadShader(nullptr, config.get<std::string_view>("/views/glassShader").data())),
    // timeLoc(GetShaderLocation(glass, "time"))
}

void MapView::update(const AircraftState &state, const float dt) {
    if (IsKeyPressed(KEY_Z)) zoom += 0.2f;
    if (IsKeyPressed(KEY_X)) zoom -= 0.2f;
    if (zoom > 5.0f) zoom = 5.0f;
    if (zoom < 0.1f) zoom = 0.1f;

    // todo find the ratio between the scene and the map...62.5f
    // map pos
    const float mapX = (state.position.x - state.mapOffset.x) / 62.5f;
    const float mapZ = (state.position.z - state.mapOffset.y) / 62.5f;

    // camera look at the player on the map
    mapCamera.target = (Vector2){mapX, mapZ};
    // todo the center of the view
    // mapCamera.offset = (Vector2){522.0f + 80.0f, 622.0f + 67.0f}; // todo even I don't remember why...
    mapCamera.offset = (Vector2){522.0f + 80.0f, 622.0f + 67.0f}; // put the camera looking at the center todo move to draw
    mapCamera.rotation = 0.0f;
    mapCamera.zoom = zoom;

    // the heading on the map
    const Vector3 forward = state.orientation.forward;
    heading = 180.0f - atan2f(forward.x, forward.z) * RAD2DEG;
}

void MapView::draw(const Vector2 &location) {
    // const auto currentTime = static_cast<float>(GetTime());
    // SetShaderValue(glass, timeLoc, &currentTime, SHADER_UNIFORM_FLOAT);
    const auto x = static_cast<int>(location.x);
    const auto y = static_cast<int>(location.y);
    constexpr int size = 150;
    constexpr float halfSize = 75.0;

    mapCamera.offset = (Vector2){location.x + halfSize, location.y + halfSize}; // put the camera looking at the center todo move to draw

    // @formatter:off
    BeginScissorMode(x, y, size, size);
        DrawRectangle(x, y, size, size, GRAY);
        BeginMode2D(mapCamera);
            DrawTexture(tex, 0, 0, WHITE);

            // draw small triangle
            // we divide by zoom to keep it the same size
            const Vector2 planeMapPos = mapCamera.target;
            const float width = 5.0f / zoom;
            const float height = 12.0f / zoom;
            Vector2 v1 = {0.0f, -height / 2.0f};
            Vector2 v2 = {-width / 2.0f, height / 2.0f};
            Vector2 v3 = {width / 2.0f, height / 2.0f};

            v1 = Vector2Add(Vector2Rotate(v1, heading * DEG2RAD), planeMapPos);
            v2 = Vector2Add(Vector2Rotate(v2, heading * DEG2RAD), planeMapPos);
            v3 = Vector2Add(Vector2Rotate(v3, heading * DEG2RAD), planeMapPos);
            DrawTriangle(v1, v2, v3, GREEN);
            DrawTriangleLines(v1, v2, v3, BLACK);


        EndMode2D();
        DrawText(
            TextFormat("Z: %.1f", zoom),
            x + 5,
            y + 140,
            10,
            BLACK
        );
    EndScissorMode();
    // @formatter:on
    /*
    BeginScissorMode(522, 622, 160, 134);
    // ClearBackground(BLACK);
    DrawRectangle(522, 622, 160, 134, GRAY);
    // BeginShaderMode(glass);
    BeginMode2D(mapCamera);
    DrawTexture(tex, 0, 0, WHITE);

    const Vector2 planeMapPos = mapCamera.target;
    const float width = 5.0f / zoom;
    const float height = 12.0f / zoom;
    Vector2 v1 = {0.0f, -height / 2.0f};
    Vector2 v2 = {-width / 2.0f, height / 2.0f};
    Vector2 v3 = {width / 2.0f, height / 2.0f};

    v1 = Vector2Add(Vector2Rotate(v1, heading * DEG2RAD), planeMapPos);
    v2 = Vector2Add(Vector2Rotate(v2, heading * DEG2RAD), planeMapPos);
    v3 = Vector2Add(Vector2Rotate(v3, heading * DEG2RAD), planeMapPos);

    DrawTriangle(v1, v2, v3, GREEN);
    DrawTriangleLines(v1, v2, v3, BLACK);
    EndMode2D();
    // EndShaderMode();
    EndScissorMode();
    */
}
