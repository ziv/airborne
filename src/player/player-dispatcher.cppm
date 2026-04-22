module;
#include <entt/entt.hpp>

export module Player:Dispatcher;

import Accessors;
import Types;
import :Config;
import :Position;
import :Rotation;
import :GroundCheck;
import :Controls;
import :Physics;
import :Camera;

export class PlayerDispatcher {
 public:
  PlayerCamera playerCamera;
  PlayerGroundCheck playerGroundCheck;

  explicit PlayerDispatcher(entt::registry& registry) : playerCamera(get_config(registry).player.camera) {};

  void update(entt::registry& reg, const float dt) {
    playerCamera.update(reg, dt);
    playerGroundCheck.update(reg, dt);
  }
};
