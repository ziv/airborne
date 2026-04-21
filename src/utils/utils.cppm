module;

export module Utils;

export namespace utils {

float approach(const float current, const float target, const float maxDelta) {
  if (current < target) return current + maxDelta > target ? target : current + maxDelta;
  if (current > target) return current - maxDelta < target ? target : current - maxDelta;
  return current;
}
}  // namespace utils