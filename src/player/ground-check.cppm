module;
#include <entt/entt.hpp>
#include "../lib/ray.hpp"

export module Player:GroundCheck;

import Components;
import :Config;

export class PlayerGroundCheck {
    PlayerGroundCheckConfig conf;

public:
    explicit PlayerGroundCheck(const PlayerGroundCheckConfig &c) : conf(c) {
    }

    void update(entt::registry &registry, const float dt) const {
        for (const auto view = registry.view<Player, TouchDown>(); auto [entity, player]: view.each()) {
            TraceLog(LOG_DEBUG, "PlayerGroundCheck: checking landing conditions for player entity %d", entity);
            const float fwdY = std::clamp(player.forward.y, -1.0f, 1.0f);
            const float rightY = std::clamp(player.right.y, -1.0f, 1.0f);

            const float pitchDeg = asinf(fwdY) * RAD2DEG;
            const float rollDeg = asinf(rightY) * RAD2DEG;

            const bool wingsLevel = fabsf(rollDeg) < 10.0f;
            const bool noseOk = pitchDeg > -5.0f && pitchDeg < 20.0f;
            const bool slowEnough = player.speed < conf.stallSpeed * conf.stallSpeed * 2.0f;

            // todo check sink rate

            if (!wingsLevel) TraceLog(LOG_WARNING, "Landing failed: wings not level (roll = %.1f°)", rollDeg);
            if (!noseOk) TraceLog(LOG_WARNING, "Landing failed: bad pitch angle (pitch = %.1f°)", pitchDeg);
            if (!slowEnough) TraceLog(LOG_WARNING, "Landing failed: too fast (speed = %.1f m/s)", player.speed);

            if (!wingsLevel || !noseOk || !slowEnough) {
                if (!registry.all_of<Crashed>(entity)) {
                    registry.emplace<Crashed>(entity);
                    TraceLog(LOG_DEBUG, "[Crashed] tag added to player");
                }
            }
            registry.erase<TouchDown>(entity);
            TraceLog(LOG_DEBUG, "[TouchDown] tag removed from player");
        }
    }
};

// export void PlayerGroundCheck(entt::registry &registry, float stallSpeed = 65.0f) {
//     for (const auto view = registry.view<Player, TouchDown>(); auto [entity, player]: view.each()) {
//         TraceLog(LOG_DEBUG, "PlayerGroundCheck: checking landing conditions for player entity %d", entity);
//         const float fwdY = std::clamp(player.forward.y, -1.0f, 1.0f);
//         const float rightY = std::clamp(player.right.y, -1.0f, 1.0f);
//
//         const float pitchDeg = asinf(fwdY) * RAD2DEG;
//         const float rollDeg = asinf(rightY) * RAD2DEG;
//
//         const bool wingsLevel = fabsf(rollDeg) < 10.0f;
//         const bool noseOk = pitchDeg > -5.0f && pitchDeg < 20.0f;
//         const bool slowEnough = player.speed < stallSpeed * stallSpeed * 2.0f;
//
//         // todo check sink rate
//
//         if (!wingsLevel) TraceLog(LOG_WARNING, "Landing failed: wings not level (roll = %.1f°)", rollDeg);
//         if (!noseOk) TraceLog(LOG_WARNING, "Landing failed: bad pitch angle (pitch = %.1f°)", pitchDeg);
//         if (!slowEnough) TraceLog(LOG_WARNING, "Landing failed: too fast (speed = %.1f m/s)", player.speed);
//
//         if (!wingsLevel || !noseOk || !slowEnough) {
//             if (!registry.all_of<Crashed>(entity)) {
//                 registry.emplace<Crashed>(entity);
//                 TraceLog(LOG_DEBUG, "[Crashed] tag added to player");
//             }
//         }
//         registry.erase<TouchDown>(entity);
//         TraceLog(LOG_DEBUG, "[TouchDown] tag removed from player");
//     }
// }
