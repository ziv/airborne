#pragma once
#include "../primitives/View.h"

class DebugView: public View {
public:
    explicit DebugView(AppConfig &inputConfig) : View(inputConfig) {
    }
    void update(const GameData &game) override;

    void draw(const GameData &game) override;
};
