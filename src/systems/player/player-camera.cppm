module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module PlayerSystems:Camera;

import Components;
import Accessors;
import Accessors;
import Types;
import Helpers;

export namespace player_systems {

void camera(entt::registry &registry, Camera &camera) {
  // static const PlayerCameraConfig &conf = get_config(registry).player.camera;
  // const Player &player = get_player(registry);
  // Camera &camera = get_camera(registry);
  //
  // camera.position = player.pos;
  // const Quaternion qTilt = QuaternionFromAxisAngle(player.right, -conf.tilt);
  // camera.target = camera.position + Vector3RotateByQuaternion(player.forward, qTilt);
  // camera.up = Vector3RotateByQuaternion(player.up, qTilt);
}
}  // namespace player_systems