module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module PlayerSystems:Position;

import Components;
import Types;
import Helpers;
import ResourceManager;
import Accessors;
import TerrainStreaming;

struct LandingZoneRet {
  bool is_landing_zone;
  bool is_carrier;
  float surface_y;
};

LandingZoneRet get_landing_zone(entt::registry &registry, const Vector3 &absolute_position) {
  for (const auto view = registry.view<Landable, Position3D, Heading, LandingZone>(); const auto [entity, landable, pos, heading, zone] : view.each()) {
    // check of we are in a landing zone
    const auto [x, y, z] = absolute_position;

    if (constexpr auto height = 50; y > zone.elevation_m + height) continue;

    // is always align to 0, 90, 180 or 270
    auto width = zone.runway_width_m / 2.0f;
    auto len = zone.runway_length_m / 2.0f;
    if (heading.heading == 90.0f || heading.heading == 270.0f) std::swap(width, len);

    if (x < pos.pos.x - width || x > pos.pos.x + width) continue;
    if (z < pos.pos.z - len || z > pos.pos.z + len) continue;

    return {true, landable.carrier, zone.elevation_m};
  }
  return {false, false, 0.0f};
}

void update_large_world_offset(Player &player, const float threshold) {
  if (player.pos.x > threshold) {
    player.pos.x -= threshold;
    player.offset.x -= threshold;
  }
  if (player.pos.x < -threshold) {
    player.pos.x += threshold;
    player.offset.x += threshold;
  }
  if (player.pos.z > threshold) {
    player.pos.z -= threshold;
    player.offset.z -= threshold;
  }
  if (player.pos.z < -threshold) {
    player.pos.z += threshold;
    player.offset.z += threshold;
  }
}

export namespace player_systems {

void position(entt::registry &registry, const float dt) {
  const PlayerPositionConfig conf = get_config(registry).player.position;
  const auto entity = get_player_entity(registry);
  auto [player, gh, inputs] = registry.get<Player, GroundHeight, const PlayerInputs>(entity);

  // update position
  player.pos = player.pos + player.velocity * dt;

  update_large_world_offset(player, conf.threshold);

  const auto absolute_position = player.pos - player.offset;

  // update ground height
  gh.height = terrain_streamer::ground_height_at(registry, absolute_position);
  gh.effective_ground_height = gh.height;

  // only when gear down, we check for landing zones
  if (inputs.gear) {
    if (const auto [is_landing_zone, is_carrier, surface_y] = get_landing_zone(registry, absolute_position); is_landing_zone) {
      // update effective ground height
      gh.effective_ground_height = fmaxf(gh.height, surface_y);
      registry.emplace_or_replace<LandingZoneDef>(entity, is_landing_zone, is_carrier, surface_y);
    }
  } else {
    registry.remove<LandingZoneDef>(entity);
  }

  const auto ground_height = gh.effective_ground_height + conf.height_above_ground;

  // limit going underground/underwater
  if (player.pos.y < ground_height) {
    player.pos.y = ground_height;

    // on ground there is no more velocity down
    if (player.velocity.y < 0.0f) player.velocity.y = 0.0f;

    // if it wasn't grounded before this moment
    // it is a touchdown, we need to add grounded and touchdown tags
    // and remove the flying one
    // replace/remove to be on the safe side
    if (!registry.all_of<Grounded>(entity)) {
      registry.emplace<Grounded>(entity);
      registry.emplace<TouchDown>(entity);
      TraceLog(LOG_WARNING, "[Grounded], [TouchDown] added to player");
    }
    // in any case we are surely not flying right now
    if (registry.remove<Flying>(entity)) TraceLog(LOG_DEBUG, "[Flying] removed from player");
  }

  // this is where we are flying
  if (player.pos.y > ground_height + 1.0f) {
    if (registry.remove<Grounded>(entity)) TraceLog(LOG_DEBUG, "[Grounded] removed from player");
    if (registry.remove<TouchDown>(entity)) TraceLog(LOG_DEBUG, "[TouchDown] removed from player");
    if (!registry.all_of<Flying>(entity)) {
      registry.emplace<Flying>(entity);
      TraceLog(LOG_DEBUG, "[Flying] added to player at height %d", player.pos.y);
    }
  }

  // last, calculate again...
  player.abs_pos = player.pos - player.offset;
}
}  // namespace player_systems