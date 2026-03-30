#pragma once
#include "GameData.h"

/**
 * An abstract signature for view elements that
 * going to be renders as part of the render loop
 */
class View {
public:
    virtual ~View() = default;

    virtual void update(const GameData &game) = 0;

    virtual void draw(const GameData &game) const = 0;
};
