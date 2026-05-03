module;

export module Utils;

export namespace utils {

float approach(const float current, const float target, const float max_delta) {
  if (current < target) return current + max_delta > target ? target : current + max_delta;
  if (current > target) return current - max_delta < target ? target : current - max_delta;
  return current;
}
}  // namespace utils

namespace to {
int i(const float number) { return static_cast<int>(number); }
}  // namespace to