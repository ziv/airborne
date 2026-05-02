module;
#include "../lib/ray.hpp"

export module Helpers;

import Types;

export Vector3 world_forward() {
    return Vector3(0.0f, 0.0f, 1.0f);
}

export Vector3 world_up() {
    return Vector3(0.0f, 1.0f, 0.0f);
}

export Vector3 world_right() {
    return Vector3(-1.0f, 0.0f, 0.0f);
}

export Vector3 gravity() {
    return Vector3(0.0f, -9.81f, 0.0f);
}


// conversions

export Feet meter_to_feet(const Meter meter) {
    return meter * 3.28084f;
}

export FeetPerMinute ms_to_fpm(const MeterPerSecond meters) {
    return meters * 196.8504f;
}

export Knot ms_to_knots(const MeterPerSecond meters) {
    return meters * 1.94384f;
}

export NauticalMile meter_to_nm(const Meter meter) {
    return meter * 0.000539957f;
}

// formatters

export const char *number_suffix(const float number) {
    if (number >= 1000000.0f) {
        return TextFormat("%.1fM", number / 1000000.0f);
    }
    if (number >= 1000.0f) {
        return TextFormat("%.1fk", number / 1000.0f);
    }
    return TextFormat("%.0f", number);
}
