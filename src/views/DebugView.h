#pragma once
#include "../primitives/View.h"

class DebugView: View {
public:
    void update(const GameData &game) override;

    void draw(const GameData &game) const override;
};
