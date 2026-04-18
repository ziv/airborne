module;
#include <entt/entt.hpp>
#include "../lib/ray.hpp"

export module Player:Rotation;

import Components;
import Types;
import Helpers;
import :Config;


export class PlayerRotation {
    PlayerTransformationConfig conf;

public:
    explicit PlayerRotation(const PlayerTransformationConfig &c) : conf(c) {
    }

    void update(entt::registry &registry, const float dt) const {
        const auto entity = registry.ctx().get<PlayerEntity>().id;
        auto [player, inputs] = registry.get<Player, PlayerInputs>(entity);

        const auto speedRatio = player.speed / conf.maxSpeed;

        auto totalPitch = 0.0f;
        auto totalYaw = 0.0f;
        auto totalRoll = 0.0f;

        // flying mode rotation
        if (registry.all_of<Flying>(entity)) {
            constexpr auto speedEpsilon = 0.001f;
            // Adverse yaw: bank angle induces a yaw moment toward the lowered wing.
            // https://en.wikipedia.org/wiki/Adverse_yaw
            const auto bankInducedYaw = player.speed < speedEpsilon ? 0.0f : player.right.y * conf.bankInduceYawRatio * dt;

            // Lift-loss pitch-down: when the lift vector is no longer purely vertical the
            // aircraft tends to pitch nose-down.
            // https://en.wikipedia.org/wiki/Stall_(fluid_dynamics)
            const auto liftLossPitch = player.speed < speedEpsilon ? 0.0f : (1.0f - player.up.y) * conf.liftLossPitchRatio * dt;

            // more speed equals more steering except yaw
            totalPitch = (inputs.pitch + liftLossPitch) * speedRatio;
            totalYaw = inputs.yaw + bankInducedYaw;
            totalRoll = inputs.roll * speedRatio;

            // VLE turbulence: random shaking above max gear-extended speed
            // https://en.wikipedia.org/wiki/V_speeds#VLE
            if (inputs.gear && player.speed > conf.vleSpeed) {
                const float overSpeed = player.speed - conf.vleSpeed;
                const float turbulenceIntensity = Clamp((overSpeed * overSpeed) * 0.0001f * dt, 0.0f, 0.03f);

                totalPitch += (static_cast<float>(GetRandomValue(-100, 100)) / 100.0f) * turbulenceIntensity;
                totalYaw += (static_cast<float>(GetRandomValue(-100, 100)) / 100.0f) * turbulenceIntensity;
                totalRoll += (static_cast<float>(GetRandomValue(-100, 100)) / 100.0f) * turbulenceIntensity * 0.5f;
            }
        }

        // ground mode rotation
        else {
            // on ground pitch can be positive only, we allow negative pitch only when nose already up
            const auto pitch = inputs.pitch > 0.0f || player.forward.y > 0.0f ? inputs.pitch : 0.0f;
            totalPitch = pitch * speedRatio;
            totalYaw = inputs.yaw;
            totalRoll = 0.0f;
        }

        // apply the changes
        const auto qPitch = QuaternionFromAxisAngle(player.right, totalPitch);
        const auto qYaw = QuaternionFromAxisAngle(player.up, totalYaw);
        const auto qRoll = QuaternionFromAxisAngle(player.forward, totalRoll);

        const auto qDelta = QuaternionMultiply(qYaw, QuaternionMultiply(qPitch, qRoll));

        // rotate
        player.rotation = QuaternionNormalize(QuaternionMultiply(qDelta, player.rotation));
        player.forward = Vector3Normalize(Vector3RotateByQuaternion(world_forward(), player.rotation));
        player.up = Vector3Normalize(Vector3RotateByQuaternion(world_up(), player.rotation));
        player.right = Vector3Normalize(Vector3RotateByQuaternion(world_right(), player.rotation));
    }
};
