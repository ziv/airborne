module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module Components:Player;

import Types;

// global tag of the player
export struct PlayerEntity {
  entt::entity id;
};

// the player state
export struct Player {
  Quaternion rotation;
  Vector3 pos;
  Vector3 offset;
  Vector3 abs_pos;
  Vector3 velocity;
  Vector3 angular_velocity;  // angular velocity
  Vector3 forward;
  Vector3 up;
  Vector3 right;
  float speed;
  float ground_height;
  float effective_ground_height;
};

// player inputs state
export struct PlayerInputs {
  float pitch = 0.0f;
  float yaw = 0.0f;
  float roll = 0.0f;
  float throttle = 0.0f;
  bool gear = true;
  bool brakes = true;
  bool autopilot = false;
};

export struct GroundHeight {
  float height;
  float effectiveGroundHeight;
};