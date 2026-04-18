module;
#include <entt/entt.hpp>
#include <utility>
#include "../lib/ray.hpp"

export module Player:Position;

import Components;
import Types;
import Helpers;
import ResourceManager;
import :Config;

struct LandingZoneRet {
    bool isLandingZone;
    bool isCarrier;
    float surfaceY;
};

inline LandingZoneRet getLandingZone(entt::registry &registry,
                                     const Vector3 &position, const Vector3 &offset) {
    // update ground height
    // if there is a carrier below us, the ground height will be 12
    const auto view = registry.view<Landable, Position3D, Heading>();
    for (const auto [entity, landable, pos, heading]: view.each()) {
        // check if the carrier is below us

        // airbase/carrier position in the aircraft coordinate frame
        const auto landingPosition = pos.pos + offset;
        // TraceLog(LOG_DEBUG, "Checking landing zone for entity %d at position: (%f, %f)", entity, landingPosition.x, landingPosition.z);

        // vector from entity center to aircraft (XZ plane)
        const float dx = position.x - landingPosition.x;
        const float dz = position.z - landingPosition.z;
        // TraceLog(LOG_DEBUG, "Relative position to entity %d: dx = %f, dz = %f", entity, dx, dz);

        // half-extents: short side (width) across heading, long side along heading
        const float halfWidth = landable.carrier ? 100.0f : 200.0f;
        const float halfLength = landable.carrier ? 250.0f : 2000.0f;
        const float surfaceY = landable.carrier ? 8.0f : 0.0f;

        // rotate into the entity's heading-aligned frame
        // heading 0 → forward = +Z, heading 90 → forward = +X
        const float rad = heading.heading * DEG2RAD;
        const float cosH = cosf(rad);
        const float sinH = sinf(rad);
        const float localAlong = dx * sinH + dz * cosH; // along runway
        const float localAcross = dx * cosH - dz * sinH; // across runway
        // TraceLog(LOG_DEBUG, "Local coordinates relative to entity %d: along = %f, across = %f", entity, localAlong, localAcross);

        // 2D footprint check
        if (fabsf(localAlong) >= halfLength) continue;
        if (fabsf(localAcross) >= halfWidth) continue;

        // vertical check — aircraft must be inside the 3D box:
        // bottom = surfaceY,  top = surfaceY + LANDING_BOX_HEIGHT
        if (constexpr float LANDING_BOX_HEIGHT = 150.0f; position.y > surfaceY + LANDING_BOX_HEIGHT) continue;
        // TraceLog(LOG_DEBUG, "Aircraft is inside landing zone of entity %d", entity);
        return {true, landable.carrier, surfaceY};
    }
    return {false, false, 0.0f};
}


float getEffectiveHeight(entt::registry &registry,
                         const PlayerPositionConfig &conf,
                         const Player &player) {
    // todo this is unsage access!!!!
    // auto &assets = registry.ctx().get<ResourceManager>();
    // const auto imageId = entt::hashed_string(conf.heightPath.c_str());
    // const auto heightMapRes = assets.images[imageId];
    //
    // const auto pos = player.pos - player.offset;
    //
    // // 125 is the ratio between the large area and the map we check the height
    // const auto x = static_cast<int>((pos.x) / conf.heightMapSizeRatio);
    // const auto z = static_cast<int>((pos.z) / conf.heightMapSizeRatio);
    //
    // // if the x and z are in the image pixels range
    // if (x < 0 || z < 0 || x >= heightMapRes->image.height || z >= heightMapRes->image.width) {
    //     return 0.0f;
    // }
    // const auto r = static_cast<float>(GetImageColor(heightMapRes->image, x, z).r);
    // return conf.maxRelativeHeight * r / 255.0f;
    return 0.0f;
}

export class PlayerPosition {
    PlayerPositionConfig conf;

public:
    explicit PlayerPosition(PlayerPositionConfig c) : conf(std::move(c)) {
    }

    void update(entt::registry &registry, const float dt) const {
        const auto entity = registry.ctx().get<PlayerEntity>().id;
        auto [player, gh] = registry.get<Player, GroundHeight>(entity);

        // current status
        const auto wasFlying = player.pos.y > gh.effectiveGroundHeight;

        // update position
        player.pos = player.pos + (player.velocity * dt);

        const auto THRESHOLD = conf.threshold;
        // update offset (large numbers)
        if (player.pos.x > THRESHOLD) {
            player.pos.x -= THRESHOLD;
            player.offset.x -= THRESHOLD;
        } else if (player.pos.x < -THRESHOLD) {
            player.pos.x += THRESHOLD;
            player.offset.x += THRESHOLD;
        }
        if (player.pos.z > THRESHOLD) {
            player.pos.z -= THRESHOLD;
            player.offset.z -= THRESHOLD;
        } else if (player.pos.z < -THRESHOLD) {
            player.pos.z += THRESHOLD;
            player.offset.z += THRESHOLD;
        }

        // keep offset globally
        registry.ctx().insert_or_assign<Offset>(Offset{player.offset});

        // update ground height
        gh.height = getEffectiveHeight(registry, conf, player);

        // are we above a landing zone? (carrier is more than ground height - sea level in this case)
        const LandingZoneRet lz = getLandingZone(registry, player.pos, player.offset);

        // if we are in a landing zone, add it to the player
        if (lz.isLandingZone && !registry.all_of<LandingZone>(entity)) {
            registry.emplace<LandingZone>(entity, lz.isLandingZone, lz.isCarrier, lz.surfaceY);
        }

        // update effective ground height
        gh.effectiveGroundHeight = lz.isLandingZone ? fmaxf(gh.height, lz.surfaceY) : gh.height;

        // limit going underground/underwater
        if (player.pos.y < gh.effectiveGroundHeight + conf.heightAboveGround) {
            player.pos.y = gh.effectiveGroundHeight + conf.heightAboveGround;

            // on ground there is no more velocity down
            if (player.velocity.y < 0.0f) player.velocity.y = 0.0f;

            // if it wasn't grounded before this moment
            // it is a touchdown, we need to add grounded and touchdown tags
            // and remove the flying one
            // replace/remove to be on the safe side
            if (!registry.all_of<Grounded>(entity)) {
                registry.emplace<Grounded>(entity);
                registry.emplace<TouchDown>(entity);
                // registry.remove<Flying>(entity);
                TraceLog(LOG_WARNING, "[Grounded], [TouchDown] added to player");
            }

            if (registry.remove<Flying>(entity)) TraceLog(LOG_WARNING, "[Grounded] removed from player");
        }

        if (player.pos.y > gh.effectiveGroundHeight + conf.heightAboveGround + 1.0f) {
            if (registry.remove<Grounded>(entity)) TraceLog(LOG_WARNING, "[Grounded] removed from player");
            if (registry.remove<TouchDown>(entity)) TraceLog(LOG_WARNING, "[TouchDown] removed from player");

            if (!registry.all_of<Flying>(entity)) {
                registry.emplace<Flying>(entity);
                TraceLog(LOG_WARNING, "[Flying] added to player");
            }
        }
    }
};
