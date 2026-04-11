#pragma once
#include "json.hpp"
#include "raylib.h"

// this is not good since we keep vectors as array, so we provide the functions
// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector3, x, y, z);

inline void from_json(const nlohmann::json &j, Vector3 &v) {
    v.x = j.at(0).get<float>();
    v.y = j.at(1).get<float>();
    v.z = j.at(2).get<float>();
}

inline void to_json(nlohmann::json &j, const Vector3 &v) {
    j = nlohmann::json::array({v.x, v.y, v.z});
}
