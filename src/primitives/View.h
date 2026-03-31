#pragma once
#include "GameData.h"

/**
 * An abstract signature for view elements that
 * going to be renders as part of the render loop
 */
class View {
    AppConfig &config;

public:
    explicit View(AppConfig &inputConfig) : config(inputConfig) {
    }

    virtual ~View() = default;

    virtual void update(const GameData &game) = 0;

    virtual void draw(const GameData &game) = 0;
};
