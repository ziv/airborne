module;
#include "../../lib/ray.hpp"

export module Npc:Steering;

import Components;

export namespace npc_steering {

struct SteerResult {
  float pitch = 0.0f;  // [-1, 1]
  float roll  = 0.0f;  // [-1, 1]
  float yaw   = 0.0f;  // [-1, 1]
};

// Returns normalised pitch/roll/yaw errors to steer 'orient' toward 'target_dir'.
// target_dir must be a unit vector in world space.
SteerResult toward(const AircraftOrientation& orient, const Vector3& target_dir) {
  const float pitch_err = -Vector3DotProduct(target_dir, orient.up);
  const float roll_err  =  Vector3DotProduct(target_dir, orient.right);
  const float yaw_err   =  Vector3DotProduct(target_dir, orient.right);

  auto clamp1 = [](float v) { return v < -1.0f ? -1.0f : (v > 1.0f ? 1.0f : v); };
  return {clamp1(pitch_err), clamp1(roll_err), clamp1(yaw_err * 0.3f)};
}

}  // namespace npc_steering
