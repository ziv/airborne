#include "MapView.h"
#include "raylib.h"
#include "raymath.h"

MapView::MapView(const AppConfig &config) : tex(LoadTexture(config.get<std::string_view>("/views/mapViewTexture").data())),
                                            glass(LoadShader(nullptr, config.get<std::string_view>("/views/glassShader").data())),
                                            timeLoc(GetShaderLocation(glass, "time")) {
}

void MapView::update(const AircraftState &state) {
    if (IsKeyPressed(KEY_Z)) zoom += 0.5f;
    if (IsKeyPressed(KEY_X)) zoom -= 0.5f;
    if (zoom > 4.0f) zoom = 4.0f;
    if (zoom < 0.5f) zoom = 0.5f;
    // zoom = Clamp(zoom, 0.5f, 4.0f);

    // todo find the ratio between the scene and the map...
    // map pos
    const float mapX = state.position.x / 64.0f;
    const float mapZ = state.position.z / 64.0f;

    // camera look at the player on the map
    mapCamera.target = (Vector2){mapX, mapZ};
    mapCamera.offset = (Vector2){522.0f + 79.0f, 622.0f + 67.0f}; // todo even I don't remember why...
    mapCamera.rotation = 0.0f;
    mapCamera.zoom = zoom;

    // the heading on the map
    const Vector3 forward = state.orientation.forward;
    heading = 180.0f - atan2f(forward.x, forward.z) * RAD2DEG;
}

void MapView::draw() {
    const auto currentTime = static_cast<float>(GetTime());
    SetShaderValue(glass, timeLoc, &currentTime, SHADER_UNIFORM_FLOAT);

    BeginScissorMode(522, 622, 160, 134);
    // ClearBackground(BLACK);
    DrawRectangle(522, 622, 160, 134, GRAY);
    BeginShaderMode(glass);
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
    EndShaderMode();
    EndScissorMode();
}
