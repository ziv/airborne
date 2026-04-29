module;
#include "../lib/ray.hpp"

export module Loadings;

import Components;

export namespace utils {
// void loading_screen(const float current) {
//
//   const auto window_width = GetScreenWidth();
//   const auto window_height = GetScreenHeight();
//   const auto loading_rectangle_width = window_width / 2;
//   const auto loading_rectangle_height = 20;
//   const auto loading_rect_x = loading_rectangle_width / 2;
//   const auto loading_rect_y = loading_rectangle_height / 2;
//
//   DrawRectangle(0, 0, window_width, window_height, BLACK);
//   DrawText("Loading...", loading_rect_x, loading_rect_y - 30, 20, WHITE);
//   DrawRectangle(
//       loading_rect_x, loading_rect_y,
//       static_cast<int>(static_cast<float>(loading_rectangle_width) * current),
//       loading_rectangle_height, GREEN);
// }
} // namespace utils