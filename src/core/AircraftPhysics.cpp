/**
 * @file AircraftPhysics.cpp
 * @brief Force-based flight dynamics implementation.
 */
#include "AircraftPhysics.h"
#include "../primitives/Constants.h"
#include "raymath.h"

AircraftPhysics::AircraftPhysics(const AppConfig &config) : conf(config.get<AircraftPhysicsConfig>("/airplane")) {
}

void AircraftPhysics::update(AircraftState &state, const float dt) const {
    state.forces.thrust = state.controls.throttle * conf.engineThrust;
    state.forces.drag = (state.forces.speed * state.forces.speed) * conf.dragCoefficient;
    state.forces.lift = (state.forces.speed * state.forces.speed) * conf.liftCoefficient;

    // todo if touch carrier should stop immediately

    // fuel consumption: idle baseline + linear at military power, exponential in afterburner
    // todo should come from configuration
    float fuelBurnRate = 0.1f;
    if (state.controls.throttle <= 1.0f) {
        fuelBurnRate += state.controls.throttle * 1.0f;
    } else {
        const float abDepth = (state.controls.throttle - 1.0f) / 0.2f;
        fuelBurnRate = 1.1f + (abDepth * 6.4f);
    }

    state.fuel -= fuelBurnRate * dt;
    if (state.fuel <= 0.0f) {
        TraceLog(LOG_WARNING, "Out of fuel!");
        state.fuel = 0.0f;
        state.controls.throttle = 0.0f;
    }

    // --- Airborne vs Ground drag model ---
    // todo move out?
    if (state.flying) {
        if (state.controls.brakes) state.forces.drag *= conf.flyingBrakesDragRatio; // breaks increase drag by 200%
        if (state.controls.gear) state.forces.drag *= conf.flyingGearDragRatio; // gear generate drag
        if (state.forces.speed < conf.stallSpeed) state.forces.lift *= conf.stallLiftRatio; // stall reduce lift by 90%
    } else {
        if (state.controls.brakes) state.forces.drag *= conf.groundBrakesDragRatio; // on ground, drag mimic the wheels brakes
        const float dampFactor = powf(0.9f, dt * 60.0f); // normalized to 60fps baseline
        if (state.controls.brakes && state.forces.speed < conf.groundBrakesSpeed) state.forces.velocity = state.forces.velocity * dampFactor;
        if (state.forces.velocity.y < 0.0f) state.forces.velocity.y = 0.0f; // on ground there is no more velocity down
    }

    const float mass = conf.weight / 9.81f;

    // --- Force vectors (world space) ---
    const auto thrustForce = state.orientation.forward * state.forces.thrust;
    const auto dragForce = state.orientation.forward * -state.forces.drag;
    const auto liftForce = state.orientation.up * state.forces.lift;
    const auto weightForce = GamePhysics::Gravity * mass;

    const auto total = thrustForce + dragForce + weightForce + liftForce;
    const auto acceleration = total / mass;

    // --- Euler integration ---
    state.forces.velocity = state.forces.velocity + (acceleration * dt);
    state.forces.speed = Vector3Length(state.forces.velocity);

    // hard speed cap (normally drag balances thrust before this limit)
    if (state.forces.speed > conf.maxSpeed && state.forces.speed != 0.0f) {
        state.forces.velocity = state.forces.velocity * conf.maxSpeed / state.forces.speed;
        state.forces.speed = Vector3Length(state.forces.velocity);
    }

    // --- Weathervaning: align velocity toward the nose above stall speed ---
    // https://en.wikipedia.org/wiki/Weathervane_effect
    if (state.forces.speed > conf.stallSpeed) {
        auto [x, y, z] = state.orientation.forward * state.forces.speed;
        state.forces.velocity.x = Lerp(state.forces.velocity.x, x, 2.0f * dt);
        state.forces.velocity.y = Lerp(state.forces.velocity.y, y, 2.0f * dt);
        state.forces.velocity.z = Lerp(state.forces.velocity.z, z, 2.0f * dt);
    }

    // update position
    state.position = state.position + (state.forces.velocity * dt);

    // --- ground / surface clamping ---
    // effectiveFloorHeight is set by GameData each frame:
    //   - over a landing zone → max(terrain, deck) + wheel clearance
    //   - elsewhere           → terrain height + wheel clearance
    if (state.position.y < state.effectiveFloorHeight) {
        state.position.y = state.effectiveFloorHeight;
        // cancel any remaining downward velocity so the aircraft rests on the surface
        if (state.forces.velocity.y < 0.0f) state.forces.velocity.y = 0.0f;
    }

    // --- Large-world coordinate re-centering ---
    // Shift position back toward the origin when it drifts too far,
    // accumulating the offset in mapOffset so world-space calculations remain correct.
    const auto ST = state.SHIFT_THRESHOLD;
    if (state.position.x > ST) {
        state.position.x -= ST;
        state.mapOffset.x -= ST;
    } else if (state.position.x < -ST) {
        state.position.x += ST;
        state.mapOffset.x += ST;
    }
    if (state.position.z > ST) {
        state.position.z -= ST;
        state.mapOffset.y -= ST;
    } else if (state.position.z < -ST) {
        state.position.z += ST;
        state.mapOffset.y += ST;
    }
}
