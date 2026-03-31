#include "GaugesView.h"
#include "../primitives/Utils.h"
#include "../lib/json.hpp"

GaugesView::GaugesView(AppConfig &inputConfig) : View(inputConfig),
                                                 sprites(LoadTexture("res/sprites.png")) {
    const json localConfig = UtilsLoaders::LoadJson("res/sprites.json");
    for (auto &[name, coords]: localConfig.items()) {
        map[name] = {
            coords[0].get<float>(), // x
            coords[1].get<float>(), // y
            coords[2].get<float>(), // width
            coords[3].get<float>() // height
        };
    }
}

void GaugesView::update(const GameData &game) {
    // nothing to do, yet
}

void GaugesView::draw(const GameData &game) {
    drawPower(game);
    drawBreaks(game);
    drawGear(game);
}


void GaugesView::drawSprite(const std::string &name, const Vector2 position) {
    if (const auto it = map.find(name); it != map.end()) {
        DrawTextureRec(sprites, it->second, position, WHITE);
    }
}

void GaugesView::drawPower(const GameData &game) {
    constexpr auto x = 370;
    constexpr auto y = 460;

    if (game.throttle == 0.0f) {
        drawSprite("power-gauge-off", Vector2{x, y - 60});
        drawSprite("power-gauge-off", Vector2{x, y - 45});
        drawSprite("power-gauge-off", Vector2{x, y - 30});
        drawSprite("power-gauge-off", Vector2{x, y - 15});
        drawSprite("power-gauge-off", Vector2{x, y});
    } else if (game.throttle < 0.2f) {
        drawSprite("power-gauge-off", Vector2{x, y - 60});
        drawSprite("power-gauge-off", Vector2{x, y - 45});
        drawSprite("power-gauge-off", Vector2{x, y - 30});
        drawSprite("power-gauge-off", Vector2{x, y - 15});
        drawSprite("power-gauge-green", Vector2{x, y});
    } else if (game.throttle < 0.4f) {
        drawSprite("power-gauge-off", Vector2{x, y - 60});
        drawSprite("power-gauge-off", Vector2{x, y - 45});
        drawSprite("power-gauge-off", Vector2{x, y - 30});
        drawSprite("power-gauge-green", Vector2{x, y - 15});
        drawSprite("power-gauge-green", Vector2{x, y});
    } else if (game.throttle < 0.6f) {
        drawSprite("power-gauge-off", Vector2{x, y - 60});
        drawSprite("power-gauge-off", Vector2{x, y - 45});
        drawSprite("power-gauge-yellow", Vector2{x, y - 30});
        drawSprite("power-gauge-green", Vector2{x, y - 15});
        drawSprite("power-gauge-green", Vector2{x, y});
    } else if (game.throttle < 0.8f) {
        drawSprite("power-gauge-off", Vector2{x, y - 60});
        drawSprite("power-gauge-yellow", Vector2{x, y - 45});
        drawSprite("power-gauge-yellow", Vector2{x, y - 30});
        drawSprite("power-gauge-green", Vector2{x, y - 15});
        drawSprite("power-gauge-green", Vector2{x, y});
    } else if (game.throttle < 1.0f) {
        drawSprite("power-gauge-yellow", Vector2{x, y - 60});
        drawSprite("power-gauge-yellow", Vector2{x, y - 45});
        drawSprite("power-gauge-yellow", Vector2{x, y - 30});
        drawSprite("power-gauge-green", Vector2{x, y - 15});
        drawSprite("power-gauge-green", Vector2{x, y});
    } else {
        drawSprite("power-gauge-red", Vector2{x, y - 60});
        drawSprite("power-gauge-yellow", Vector2{x, y - 45});
        drawSprite("power-gauge-yellow", Vector2{x, y - 30});
        drawSprite("power-gauge-green", Vector2{x, y - 15});
        drawSprite("power-gauge-green", Vector2{x, y});
    }

    DrawRectangle(406, y - 15, 64, 30, DARKGRAY);
    DrawText(TextFormat("t %i%%", static_cast<int>(game.throttle * 100)), 420, y - 5, 15, GREEN);
}

void GaugesView::drawBreaks(const GameData &game) {
    if (game.breaks) drawSprite("breaks-on", Vector2{952, 761});
    else drawSprite("breaks-off", Vector2{952, 761});
}

void GaugesView::drawGear(const GameData &game) {
    if (game.gear) drawSprite("gear-on", Vector2{1011, 765});
    else drawSprite("gear-off", Vector2{1011, 765});
}

GaugesView::~GaugesView() {
    UnloadTexture(sprites);
}
