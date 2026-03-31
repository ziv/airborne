#include "GaugesView.h"

GaugesView::GaugesView(AppConfig &inputConfig) : View(inputConfig),
                                                 sprites(LoadTexture("res/sprites.png")) {
}

void GaugesView::update(const GameData &game) {
    // nothing to do, yet
}

void GaugesView::draw(const GameData &game) const {
    drawPower(game);
}

void GaugesView::drawPower(const GameData &game) const {
    constexpr auto x = 370;
    constexpr auto y = 460;

    if (game.throttle == 0.0f) {
        DrawTexture(off, x, y - 60, WHITE);
        DrawTexture(off, x, y - 45, WHITE);
        DrawTexture(off, x, y - 30, WHITE);
        DrawTexture(off, x, y - 15, WHITE);
        DrawTexture(off, x, y, WHITE);
    } else if (game.throttle < 0.2f) {
        DrawTexture(off, x, y - 60, WHITE);
        DrawTexture(off, x, y - 45, WHITE);
        DrawTexture(off, x, y - 30, WHITE);
        DrawTexture(off, x, y - 15, WHITE);
        DrawTexture(green, x, y, WHITE);
    } else if (game.throttle < 0.4f) {
        DrawTexture(off, x, y - 60, WHITE);
        DrawTexture(off, x, y - 45, WHITE);
        DrawTexture(off, x, y - 30, WHITE);
        DrawTexture(green, x, y - 15, WHITE);
        DrawTexture(green, x, y, WHITE);
    } else if (game.throttle < 0.6f) {
        DrawTexture(off, x, y - 60, WHITE);
        DrawTexture(off, x, y - 45, WHITE);
        DrawTexture(yellow, x, y - 30, WHITE);
        DrawTexture(green, x, y - 15, WHITE);
        DrawTexture(green, x, y, WHITE);
    } else if (game.throttle < 0.8f) {
        DrawTexture(off, x, y - 60, WHITE);
        DrawTexture(yellow, x, y - 45, WHITE);
        DrawTexture(yellow, x, y - 30, WHITE);
        DrawTexture(green, x, y - 15, WHITE);
        DrawTexture(green, x, y, WHITE);
    } else if (game.throttle < 1.0f) {
        DrawTexture(yellow, x, y - 60, WHITE);
        DrawTexture(yellow, x, y - 45, WHITE);
        DrawTexture(yellow, x, y - 30, WHITE);
        DrawTexture(green, x, y - 15, WHITE);
        DrawTexture(green, x, y, WHITE);
    } else {
        DrawTexture(red, x, y - 60, WHITE);
        DrawTexture(yellow, x, y - 45, WHITE);
        DrawTexture(green, x, y - 30, WHITE);
        DrawTexture(green, x, y - 15, WHITE);
        DrawTexture(green, x, y, WHITE);
    }

    DrawRectangle(406, y - 15, 64, 30, DARKGRAY);
    DrawText(TextFormat("t %i%%", static_cast<int>(game.throttle * 100)), 420, y - 5, 15, GREEN);
}

GaugesView::~GaugesView() {
    UnloadTexture(off);
    UnloadTexture(red);
    UnloadTexture(green);
    UnloadTexture(yellow);
    UnloadTexture(sprites);
}
