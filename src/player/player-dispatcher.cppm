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

export class PlayerDispatcher
{
public:
  PlayerPhysics playerPhysics;
  PlayerPosition playerPosition;
  PlayerControls playerControls;
  PlayerCamera playerCamera;
  PlayerGroundCheck playerGroundCheck;
  PlayerRotation playerRotation;

  explicit PlayerDispatcher(entt::registry& registry, const JsonConfig& cfg)
    : playerPhysics(get_config(registry).player.aircraft)
    , playerPosition(get_config(registry).player.position)
    , playerCamera(get_config(registry).player.camera) {};

  void update(entt::registry& reg, const float dt)
  {
    playerControls.update(reg, dt);
    playerPhysics.update(reg, dt);
    playerPosition.update(reg, dt);
    playerRotation.update(reg, dt);
    playerCamera.update(reg, dt);
    playerGroundCheck.update(reg, dt);
  }
};
