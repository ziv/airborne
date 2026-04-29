module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module PlayerSystems:Camera;

import Components;
import Accessors;
import Types;
import Helpers;

export namespace player_systems {

void camera(entt::registry &registry, Camera &camera) {
  // static const PlayerCameraConfig &conf = get_config(registry).player.camera;
  auto &options = registry.ctx().get<GameOptions>();
  const Player &player = get_player(registry);

  camera.fovy = options.fov;
  camera.position = player.pos;
  const Quaternion qTilt = QuaternionFromAxisAngle(player.right, -options.tilt);
  camera.target = camera.position + Vector3RotateByQuaternion(player.forward, qTilt);
  camera.up = Vector3RotateByQuaternion(player.up, qTilt);
}
}  // namespace player_systems