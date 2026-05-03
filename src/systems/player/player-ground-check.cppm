module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module PlayerSystems:GroundCheck;

import Components;
import Accessors;

export namespace player_systems {
void ground_check(entt::registry& registry, const float dt) {
  // const auto& stall_speed = get_config(registry).player.aircraft.stall_speed;

  for (const auto view = registry.view<Player, TouchDown>(); auto [entity, player] : view.each()) {
    TraceLog(LOG_DEBUG, "checking landing conditions for player entity");
    const float fwdY = std::clamp(player.forward.y, -1.0f, 1.0f);
    const float rightY = std::clamp(player.right.y, -1.0f, 1.0f);

    const float pitch_deg = asinf(fwdY) * RAD2DEG;
    const float roll_deg = asinf(rightY) * RAD2DEG;

    const bool wings_level = fabsf(roll_deg) < 10.0f;
    const bool nose_ok = pitch_deg > -5.0f && pitch_deg < 20.0f;
    const bool slow_enough = player.speed < 65.0f * 2.0f;  // todo take from config

    // todo check sink rate

    if (!wings_level) TraceLog(LOG_DEBUG, "landing failed: wings not level (roll = %.1f°)", roll_deg);
    if (!nose_ok) TraceLog(LOG_DEBUG, "landing failed: bad pitch angle (pitch = %.1f°)", pitch_deg);
    if (!slow_enough) TraceLog(LOG_DEBUG, "landing failed: too fast (speed = %.1f m/s)", player.speed);

    if (!wings_level || !nose_ok || !slow_enough) {
      if (!registry.all_of<Crashed>(entity)) {
        registry.emplace<Crashed>(entity);
        TraceLog(LOG_DEBUG, "[Crashed] tag added to player");
      }
    }
    registry.erase<TouchDown>(entity);
    TraceLog(LOG_DEBUG, "[TouchDown] tag removed from player");
  }
}
};  // namespace player_systems