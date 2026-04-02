#pragma once
#include "../core/GameData.h"

class DebugView {
    GameData &game;

    bool display = true;

public:
    explicit DebugView(GameData &game);

    void update();

    void draw() const;
};
