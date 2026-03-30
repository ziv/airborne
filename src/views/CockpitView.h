#pragma once
#include "../primitives/View.h"


class CockpitView : View {
public:
    explicit CockpitView(AppConfig &inputConfig) : View(inputConfig) {
    }

    void update(const GameData &game) override;

    void draw(const GameData &game) const override;
};
