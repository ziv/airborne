module;
#include <entt/entt.hpp>
#include "../../lib/ray.hpp"

export module RenderSystem:Minimap;

import Components;

export void RenderMinimap(entt::registry &registry) {
    const auto view = registry.view<DashboardSlot, MinimapWidget, Position2D, WithTexture>();
    if (view.begin() == view.end()) return;

    const entt::entity entity = view.front();
    const auto [wd, pos, tex] = registry.get<MinimapWidget, Position2D, WithTexture>(entity);

    const auto player_entity = registry.ctx().get<PlayerEntity>().id;
    const auto player = registry.get<Player>(player_entity);

    const auto aircraftPosition = player.pos - player.offset;
    const auto mapX = aircraftPosition.x / wd.cfg.mapsRatio;
    const auto mapZ = aircraftPosition.z / wd.cfg.mapsRatio;

    Camera2D cam{0};
    cam.target = (Vector2){mapX, mapZ};
    cam.rotation = 0.0f;
    cam.zoom = wd.zoom;

    const auto heading = 180.0f - atan2f(player.forward.x, player.forward.z) * RAD2DEG;

    const auto x = static_cast<int>(pos.pos.x);
    const auto y = static_cast<int>(pos.pos.y);
    const int size = wd.cfg.size;
    const auto halfSize = static_cast<float>(wd.cfg.size) / 2.0f;

    cam.offset = (Vector2){pos.pos.x + halfSize, pos.pos.y + halfSize};

    BeginScissorMode(x, y, size, size);
    DrawRectangle(x, y, size, size, GRAY);
    BeginMode2D(cam);
    DrawTexture(tex.handle->res, 0, 0, WHITE);

    // draw small triangle
    // we divide by zoom to keep it the same size
    const Vector2 planeMapPos = cam.target;
    const float width = 5.0f / wd.zoom;
    const float height = 12.0f / wd.zoom;
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
        TextFormat("Z: x%.1f", wd.zoom),
        x + 5,
        y + 140,
        10,
        BLACK
    );
    EndScissorMode();

    // const auto view = registry.view<Position2D, MinimapWidget>();
    // for (auto [entity, pos, c]: view.each()) {
    //
    // }
}
