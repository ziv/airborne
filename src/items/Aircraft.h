#pragma once
#include <string_view>
#include "raylib.h"

class Aircraft {
    std::string_view name;
    Model model;

public:
    Aircraft(std::string_view name, std::string_view modelPath);
};
