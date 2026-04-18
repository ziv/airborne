#pragma once
#include <nlohmann/json.hpp>
// this file is because when both needed, they have the same definitions
#include <raylib.h>
#define RAYMATH_STATIC_INLINE
#include <raymath.h>

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector2, x, y)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector3, x, y, z)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Quaternion, x, y, z, w)

// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Color, r, g, b, a)
inline void from_json(const nlohmann::json &j, Color &c) {
    c.r = j.at(0).get<unsigned char>();
    c.g = j.at(1).get<unsigned char>();
    c.b = j.at(2).get<unsigned char>();
    c.a = j.at(3).get<unsigned char>();
}

inline void to_json(nlohmann::json &j, const Color &c) {
    j = nlohmann::json::array({c.r, c.g, c.b, c.a});
}