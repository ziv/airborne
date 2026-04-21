module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module Player:Rotation;

import Components;
import Accessors;
import Types;
import Helpers;
import :Config;

export class PlayerRotation {
  // PlayerTransformationConfig conf;

 public:
  // explicit PlayerRotation(const PlayerTransformationConfig& c) {}

  void update(entt::registry& registry, const float dt) const {
    auto& player = get_player(registry);
    const auto& inputs = get_player_inputs(registry);

    if (const float angularSpeed = Vector3Length(player.angular_velocity); angularSpeed > 0.0001f) {
      if (!is_player_flying(registry)) {
        // no roll on ground
        player.angular_velocity.z = 0.0f;
        // no nose down on ground
        // unless nose is already up
        player.angular_velocity.x = player.angular_velocity.x <= 0.0f || player.forward.y >= 0.0f ? player.angular_velocity.x : 0.0f;
      }

      const float rotationAngle = angularSpeed * dt;

      const Vector3 rotationAxis = Vector3Normalize(player.angular_velocity);
      const Quaternion deltaRotation = QuaternionFromAxisAngle(rotationAxis, rotationAngle);
      player.rotation = QuaternionMultiply(deltaRotation, player.rotation);
      player.rotation = QuaternionNormalize(player.rotation);

      player.forward = Vector3Normalize(Vector3RotateByQuaternion(world_forward(), player.rotation));
      player.up = Vector3Normalize(Vector3RotateByQuaternion(world_up(), player.rotation));
      player.right = Vector3Normalize(Vector3RotateByQuaternion(world_right(), player.rotation));
    }
  }
};
