#pragma once
#include "../core/GameData.h"

/**
 * An abstract signature for view elements that
 * going to be renders as part of the render loop
 */
template <typename Derived>
class View {
protected:
    AppConfig &config;

public:
    explicit View(AppConfig &inputConfig) : config(inputConfig) {
    }

    virtual ~View() = default;

    virtual void update(const GameData &game) = 0;

    virtual void draw(const GameData &game) = 0;

    void draw() const {
        static_cast<const Derived*>(this)->drawImpl();
    }

    void update(float deltaTime) {
        static_cast<Derived*>(this)->updateImpl(deltaTime);
    }
};
