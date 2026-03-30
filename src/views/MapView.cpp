#include "MapView.h"

MapView::~MapView() {
    UnloadTexture(map);
}

void MapView::update(const GameData &game) {
    if (IsKeyPressed(KEY_Z)) zoom += 0.5f;
    if (IsKeyPressed(KEY_X)) zoom -= 0.5f;
    if (zoom > 4.0f) zoom = 4.0f;
    if (zoom < 1.0f) zoom = 1.0f;

    // player pos
    const auto pos = game.GetPosition();

    // map pos
    const float mapX = pos.x / 64.0f;
    const float mapZ = pos.z / 64.0f;

    // camera look at the player on the map
    mapCamera.target = (Vector2){mapX, mapZ};
    mapCamera.offset = (Vector2){522.0f + 79.0f, 542.0f + 67.0f};
    mapCamera.rotation = 0.0f;
    mapCamera.zoom = zoom;

    // the heading on the map
    const Vector3 forward = game.GetForward();
    heading = 180.0f - atan2f(forward.x, forward.z) * RAD2DEG;
}

void MapView::draw(const GameData &game) const {
    BeginScissorMode(522, 542, 158, 134);
    ClearBackground(BLACK);
    BeginMode2D(mapCamera);
    DrawTexture(map, 0, 0, WHITE);

    const Vector2 planeMapPos = mapCamera.target;
    const float width = 5.0f / zoom;
    const float height = 12.0f /zoom;
    Vector2 v1 = {0.0f, -height / 2.0f};
    Vector2 v2 = {-width / 2.0f, height / 2.0f};
    Vector2 v3 = {width / 2.0f, height / 2.0f};

    v1 = Vector2Add(Vector2Rotate(v1, heading * DEG2RAD), planeMapPos);
    v2 = Vector2Add(Vector2Rotate(v2, heading * DEG2RAD), planeMapPos);
    v3 = Vector2Add(Vector2Rotate(v3, heading * DEG2RAD), planeMapPos);

    DrawTriangle(v1, v2, v3, GREEN);
    DrawTriangleLines(v1, v2, v3, BLACK);
    EndMode2D();
    EndScissorMode();
}
