module;
#include <entt/entt.hpp>
#include "../lib/ray.hpp"

export module Player:Camera;

import Components;
import Helpers;
import :Config;

export class PlayerCamera {
    PlayerCameraConfig conf;
    Camera camera = {};

public:
    explicit PlayerCamera(const PlayerCameraConfig &c) : conf(c) {
        camera.up = world_up();
        camera.fovy = conf.fov;
        camera.projection = CAMERA_PERSPECTIVE;
    }

    Camera &getCamera() { return camera; }

    void update(entt::registry &registry, const float dt) {
        for (const auto view = registry.view<Player>(); auto [entity, player]: view.each()) {
            camera.position = player.pos;
            const Quaternion qTilt = QuaternionFromAxisAngle(player.right, -conf.tilt);
            camera.target = camera.position + Vector3RotateByQuaternion(player.forward, qTilt);
            camera.up = Vector3RotateByQuaternion(player.up, qTilt);
        }
    }
};
