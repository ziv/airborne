module;
#include <entt/entt.hpp>

export module Player:Dispatcher;

import :Config;
import :Position;
import :Rotation;
import :GroundCheck;
import :Controls;
import :Physics;
import :Camera;

export class PlayerDispatcher {
public:
    PlayerControls playerControls;
    PlayerPhysics playerPhysics;
    PlayerPosition playerPosition;
    PlayerRotation playerRotation;
    PlayerCamera playerCamera;
    PlayerGroundCheck playerGroundCheck;

    explicit PlayerDispatcher(const JsonConfig &cfg)
        : playerControls(cfg.get<PlayerControlsConfig>("/player/controls")),
          playerPhysics(cfg.get<PlayerPhysicsConfig>("/player/aircraft")),
          playerPosition(cfg.get<PlayerPositionConfig>("/player/position")),
          playerRotation(cfg.get<PlayerTransformationConfig>("/player/aircraft")),
          playerCamera(cfg.get<PlayerCameraConfig>("/player/camera")),
          playerGroundCheck(cfg.get<PlayerGroundCheckConfig>("/player/groundCheck")) {
    };

    void update(entt::registry &reg, const float dt) {
        playerControls.update(reg, dt);
        playerPhysics.update(reg, dt);
        playerPosition.update(reg, dt);
        playerRotation.update(reg, dt);
        playerCamera.update(reg, dt);
        playerGroundCheck.update(reg, dt);
    }
};
