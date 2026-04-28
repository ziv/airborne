module;
#include <entt/entt.hpp>
#include <utility>

#include "../../lib/ray.hpp"

export module PlayerSystems:Position;

import Components;
import Types;
import Helpers;
import ResourceManager;
import Accessors;

struct LandingZoneRet {
  bool is_landing_zone;
  bool is_carrier;
  float surface_y;
};

float get_effective_height(entt::registry &registry, const PlayerPositionConfig &conf, const Vector3 &position, const entt::hashed_string &heightmap) {
  // todo make static?
  const auto map = get_resource_manager(registry).images[heightmap]->res;

  // 125 is the ratio between the large area and the map we check the height
  const auto x = static_cast<int>(position.x / conf.heightMapSizeRatio);
  const auto z = static_cast<int>(position.z / conf.heightMapSizeRatio);
  //
  // if the x and z are in the image pixels range
  if (x < 0 || z < 0 || x >= map.height || z >= map.width) return 0.0f;


  const auto r = static_cast<float>(GetImageColor(map, x, z).r);

  // this magic is the sea level in the north map
  // todo should enter to the configuration
  return conf.maxRelativeHeight * (r / 255.0f - 0.203922);
  // return 0.0f;
}

static LandingZoneRet get_landing_zone(entt::registry &registry, const Vector3 &absolute_position) {
  // update ground height
  // if there is a carrier below us, the ground height will be 12
  const auto view = registry.view<Landable, Position3D, Heading>();
  for (const auto [entity, landable, pos, heading] : view.each()) {
    // check if the carrier is below us

    // airbase/carrier position in the aircraft coordinate frame
    // const auto landingPosition = pos.pos + offset;
    // TraceLog(LOG_DEBUG, "Checking landing zone for entity %d at position:
    // (%f, %f)", entity, landingPosition.x, landingPosition.z);

    // vector from entity center to aircraft (XZ plane)
    const float dx = absolute_position.x - pos.pos.x;
    const float dz = absolute_position.z - pos.pos.z;
    // TraceLog(LOG_DEBUG, "Relative position to entity %d: dx = %f, dz = %f",
    // entity, dx, dz);

    // half-extents: short side (width) across heading, long side along
    // heading
    const float halfWidth = landable.carrier ? 100.0f : 200.0f;
    const float halfLength = landable.carrier ? 250.0f : 2000.0f;
    const float surfaceY = landable.carrier ? 8.0f : 0.0f;

    // rotate into the entity's heading-aligned frame
    // heading 0 → forward = +Z, heading 90 → forward = +X
    const float rad = heading.heading * DEG2RAD;
    const float cosH = cosf(rad);
    const float sinH = sinf(rad);
    const float localAlong = dx * sinH + dz * cosH;   // along runway
    const float localAcross = dx * cosH - dz * sinH;  // across runway

    // 2D footprint check
    if (fabsf(localAlong) >= halfLength) continue;
    if (fabsf(localAcross) >= halfWidth) continue;

    // vertical check — aircraft must be inside the 3D box:
    // bottom = surfaceY,  top = surfaceY + LANDING_BOX_HEIGHT
    if (constexpr float LANDING_BOX_HEIGHT = 150.0f; absolute_position.y > surfaceY + LANDING_BOX_HEIGHT) continue;
    return {true, landable.carrier, surfaceY};
  }
  return {false, false, 0.0f};
}

export namespace player_systems {

void position(entt::registry &registry, const float dt) {
  static PlayerPositionConfig conf = get_config(registry).player.position;
  static entt::hashed_string heightmap = entt::hashed_string(conf.heightPath.c_str());

  const auto entity = get_player_entity(registry);
  auto [player, gh, inputs] = registry.get<Player, GroundHeight, const PlayerInputs>(entity);

  // update position
  player.pos = player.pos + (player.velocity * dt);

  const auto THRESHOLD = conf.threshold;
  // update offset (large numbers)
  if (player.pos.x > THRESHOLD) {
    player.pos.x -= THRESHOLD;
    player.offset.x -= THRESHOLD;
  }
  if (player.pos.x < -THRESHOLD) {
    player.pos.x += THRESHOLD;
    player.offset.x += THRESHOLD;
  }
  if (player.pos.z > THRESHOLD) {
    player.pos.z -= THRESHOLD;
    player.offset.z -= THRESHOLD;
  }
  if (player.pos.z < -THRESHOLD) {
    player.pos.z += THRESHOLD;
    player.offset.z += THRESHOLD;
  }

  const auto absolute_position = player.pos - player.offset;

  // update ground height
  gh.height = get_effective_height(registry, conf, absolute_position, heightmap);

  // are we above a landing zone? (carrier is more than ground height - sea
  // level in this case)
  const LandingZoneRet lz = inputs.gear ? get_landing_zone(registry, absolute_position) : LandingZoneRet{false, false, 0.0f};

  // if we are in a landing zone, add it to the player
  if (lz.is_landing_zone && !registry.all_of<LandingZoneDef>(entity))
    registry.emplace_or_replace<LandingZoneDef>(entity, lz.is_landing_zone, lz.is_carrier, lz.surface_y);

  // if we are not in a landing zone, remove it from the user
  if (!lz.is_landing_zone && registry.all_of<LandingZoneRet>(entity)) registry.remove<LandingZoneDef>(entity);

  // update effective ground height
  gh.effectiveGroundHeight = lz.is_landing_zone ? fmaxf(gh.height, lz.surface_y) : gh.height;

  const auto ground_height = gh.effectiveGroundHeight + conf.heightAboveGround;
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

    if (registry.remove<Flying>(entity)) TraceLog(LOG_WARNING, "[Grounded] removed from player");
  }

  if (player.pos.y > ground_height + 1.0f) {
    if (registry.remove<Grounded>(entity)) TraceLog(LOG_WARNING, "[Grounded] removed from player");

    if (registry.remove<TouchDown>(entity)) TraceLog(LOG_WARNING, "[TouchDown] removed from player");

    if (!registry.all_of<Flying>(entity)) {
      registry.emplace<Flying>(entity);
      TraceLog(LOG_WARNING, "[Flying] added to player");
    }
  }
}
}  // namespace player_systems