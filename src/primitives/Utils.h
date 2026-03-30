#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>
#include "GameData.h"
#include "raylib.h"
#include "raymath.h"
#include "Constants.h"
#include "../lib/json.hpp"

using json = nlohmann::json;

inline Vector3 GetFlatForward(const Vector3 &currentForward, const Vector3 &currentUp) {
    Vector3 flatForward = {currentForward.x, 0.0f, currentForward.z};
    // edge case guard
    if (Vector3Length(flatForward) < 0.001f) {
        flatForward = {currentUp.x, 0.0f, currentUp.z};
    }
    return Vector3Normalize(flatForward);
}

inline Vector3 GetFlatRight(const Vector3 &currentForward, const Vector3 &currentUp = GamePhysics::WorldUp) {
    return Vector3Normalize(Vector3CrossProduct(currentForward, currentUp));
}

inline std::string FormatNumber(const float num) {
    std::stringstream ss;
    if (num >= 1000000) {
        ss << std::fixed << std::setprecision(2) << (num / 1000000.0) << "M";
    } else if (num >= 1000) {
        ss << std::fixed << std::setprecision(2) << (num / 1000.0) << "K";
    } else {
        ss << num;
    }
    return ss.str();
}