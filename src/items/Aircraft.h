#pragma once
#include <string_view>
#include "raylib.h"

class Aircraft {
    std::string_view name;
    // Model model;

    Quaternion rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    Vector3 position = {0.0f, 0.0f, 0.0f};
    Vector3 velocity = {0.0f, 0.0f, 0.0f};

public:
    Model model;
    Aircraft(std::string_view name, std::string_view modelPath);

    // void update();
};
