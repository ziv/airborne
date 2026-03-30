#pragma once
#include "raylib.h"

/**
 * Some definitions for our world
 */
namespace GamePhysics {
    // real world directions
    constexpr Vector3 WorldForward = {0.0f, 0.0f, 1.0f};
    constexpr Vector3 WorldUp = {0.0f, 1.0f, 0.0f};
    constexpr Vector3 WorldRight = {-1.0f, 0.0f, 0.0f};

    // the one and only
    constexpr Vector3 Gravity = {0.0f, -9.81f, 0.0f};
}
