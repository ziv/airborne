/**
 * @file GameData.cpp
 * @brief Simulation orchestration — per-frame update pipeline and landing logic.
 */
#include "GameData.h"

#include "raymath.h"
#include <cmath>

GameData::GameData(const AppConfig &config, const Scenario &scenario)
    : heightAboveGround(config.get<float>("/airplane/heightAboveGround")),
      stallSpeed(config.get<float>("/airplane/stallSpeed")),
      autopilot(config, scenario),
      scenario(scenario),
      aircraftControls(config),
      aircraftPhysics(config),
      aircraftTransformation(config),
      aircraftCamera(config) {
    // place the aircraft at the scenario start position
    state.position = scenario.start.position;
    state.fuel = scenario.start.fuel;
    spawnInitialEntities();
}

void GameData::spawnInitialEntities() {
    for (const auto &def: scenario.entities) {
        if (def.type == EntityType::STRUCTURE) {
            auto gt = std::make_unique<GroundTarget>();
            static_cast<EntityBase &>(*gt) = def;
            gt->strategicTarget = true;
            entities.spawn(std::move(gt));
        } else {
            auto e = std::make_unique<EntityBase>(def);
            entities.spawn(std::move(e));
        }
    }
}

/// Main per-frame update — runs controls, physics, orientation, camera, entities.
/// Handles landing-zone detection, ground collision, and crash logic.
void GameData::update(const float dt) {
    // once crashed, freeze everything
    if (state.crashed) return;

    // --- 1. Landing zone detection ---
    // Check if aircraft is over a friendly airstrip or carrier deck.
    state.landingZone = checkLandingZones();

    // --- 2. Effective floor height ---
    // The floor is the surface below us plus wheel clearance.
    // Over a carrier deck the surface is 150 m; otherwise use terrain height.
    if (state.landingZone.active) {
        state.effectiveFloorHeight = fmaxf(state.groundHeight, state.landingZone.surfaceHeight) + heightAboveGround;
    } else {
        state.effectiveFloorHeight = state.groundHeight + heightAboveGround;
    }

    // --- 3. Determine flying vs ground state ---
    // Remember previous state so we can detect the flying→ground transition.
    const bool wasFlying = state.flying;
    state.flying = state.position.y > state.effectiveFloorHeight + 1.0f;

    // --- 4. Controls (who is control the aricraft) ---
    if (autopilot.isActive()) autopilot.steer(state, dt);
    else aircraftControls.update(state, dt);

    // --- 5. Physics (ground-clamps to effectiveFloorHeight) ---
    aircraftPhysics.update(state, dt);

    // --- 6. Crash / landing detection ---
    // Only check at the moment the aircraft transitions from flying to ground (touchdwon).
    if (wasFlying && !state.flying) {
        // hit terrain / sea with no runway below → crash
        // landed on a strip but too fast, gear up, or wings not level → crash
        if (!state.landingZone.active || !isGoodLanding()) state.crashed = true;
        if (!state.crashed && state.landingZone.isCarrier) {
            // todo replace with physics of stoping and not like that
            // todo stop aitcraft as it touch the ground
            state.forces.velocity = {0.0f, 0.0f, 0.0f};
        }
    }

    // --- 7. Remaining subsystems ---
    aircraftTransformation.update(state, dt);
    aircraftCamera.update(state, dt);
    entities.update(state, dt);
}

/// Scan all friendly AIRBASE entities and test whether the aircraft
/// is inside their oriented landing-zone box.
///
/// The box is aligned with the entity heading:
///   - carrier:  200 m wide × 500 m long × LANDING_BOX_HEIGHT tall at deck (150 m)
///   - airbase:  500 m wide × 2000 m long × LANDING_BOX_HEIGHT tall at ground level
///
/// Returns an active LandingZoneInfo when inside a box, or an inactive one otherwise.
LandingZoneInfo GameData::checkLandingZones() const {
    // constexpr auto rangeSquared = 15000.f * 1500.0f;

    for (const auto &e: entities.allEntities()) {
        // only consider alive, friendly airbases
        if (e->type != EntityType::AIRBASE) continue;
        if (e->faction != Faction::FRIENDLY) continue;
        if (!e->isAlive()) continue;

        // convert entity world position into the aircraft's local coordinate frame
        const float entityLocalX = e->position.x + state.mapOffset.x;
        const float entityLocalZ = e->position.z + state.mapOffset.y;

        // vector from entity center to aircraft (XZ plane)
        const float dx = state.position.x - entityLocalX;
        const float dz = state.position.z - entityLocalZ;

        // todo make this test better
        if (dx + dz > 10000) continue;

        const bool isCarrier = (e->subtype == "carrier");

        // half-extents: short side (width) across heading, long side along heading
        // todo see EntityRegistry draw, GameData consts
        const float halfWidth = isCarrier ? 100.0f : 200.0f; // 200/2  or 500/2
        const float halfLength = isCarrier ? 250.0f : 2000.0f; // 500/2  or 2000/2
        const float surfaceY = isCarrier ? 8.0f : 0.0f;


        // rotate into the entity's heading-aligned frame
        // heading 0 → forward = +Z, heading 90 → forward = +X
        const float rad = e->heading * DEG2RAD;
        const float cosH = cosf(rad);
        const float sinH = sinf(rad);
        const float localAlong = dx * sinH + dz * cosH; // along runway
        const float localAcross = dx * cosH - dz * sinH; // across runway

        // 2D footprint check
        if (fabsf(localAlong) >= halfLength) continue;
        if (fabsf(localAcross) >= halfWidth) continue;

        // vertical check — aircraft must be inside the 3D box:
        //   bottom = surfaceY,  top = surfaceY + LANDING_BOX_HEIGHT
        if (state.position.y < surfaceY || state.position.y > surfaceY + LANDING_BOX_HEIGHT) continue;

        // aircraft is inside this landing zone
        return {true, isCarrier, surfaceY};
    }
    return {}; // not over any landing zone
}

/// Landing quality check — called at the exact moment of touchdown.
/// A good landing requires:
///   1. gear deployed
///   2. wings roughly level (roll < 5°)
///   3. gentle nose-up attitude (pitch between -2° and +10°)
///   4. speed below 1.5× stall speed
bool GameData::isGoodLanding() const {
    // gear must be down
    if (!state.controls.gear) return false;

    const float fwdY = std::clamp(state.orientation.forward.y, -1.0f, 1.0f);
    const float rightY = std::clamp(state.orientation.right.y, -1.0f, 1.0f);

    const float pitchDeg = asinf(fwdY) * RAD2DEG;
    const float rollDeg = asinf(rightY) * RAD2DEG;

    const bool wingsLevel = fabsf(rollDeg) < MAX_LANDING_ROLL;
    const bool noseOk = pitchDeg > MIN_LANDING_PITCH && pitchDeg < MAX_LANDING_PITCH;
    const bool slowEnough = state.forces.speed < stallSpeed * MAX_LANDING_SPEED_RATIO;

    // todo check sink rate

    if (!wingsLevel) {
        TraceLog(LOG_WARNING, "Landing failed: wings not level (roll = %.1f°)", rollDeg);
    }
    if (!noseOk) {
        TraceLog(LOG_WARNING, "Landing failed: bad pitch angle (pitch = %.1f°)", pitchDeg);
    }
    if (!slowEnough) {
        TraceLog(LOG_WARNING, "Landing failed: too fast (speed = %.1f m/s)", state.forces.speed);
    }
    return wingsLevel && noseOk && slowEnough;
}
