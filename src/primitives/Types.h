#pragma once

using NoUint = float;

constexpr NoUint operator""_nut(const long double val) {
    return static_cast<NoUint>(val);
}

using MeterPerSecond = float;

constexpr MeterPerSecond operator""_mps(const long double val) {
    return static_cast<MeterPerSecond>(val);
}

using Meter = float;

constexpr Meter operator""_mtr(const long double val) {
    return static_cast<Meter>(val);
}

using Newton = float;

constexpr Newton operator""_ntn(const long double val) {
    return static_cast<Newton>(val);
}

using Degree = float;

constexpr Degree operator""_deg(const long double val) {
    return static_cast<Degree>(val);
}

using Ratio = float;

constexpr Ratio operator""_rto(const long double val) {
    return static_cast<Ratio>(val);
}
