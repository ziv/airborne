#include "AircraftPhysics.h"
#include "../primitives/Constants.h"
#include "raymath.h"

AircraftPhysics::AircraftPhysics(const AppConfig &config) : weight(config.get<float>("/airplane/weight")),
                                                            engineThrust(config.get<float>("/airplane/engineThrust")),
                                                            maxSpeed(config.get<float>("/airplane/maxSpeed")),
                                                            stallSpeed(config.get<float>("/airplane/stallSpeed")),
                                                            groundBrakesSpeed(config.get<float>("/airplane/groundBrakesSpeed")),
                                                            dragCoefficient(config.get<float>("/airplane/dragCoefficient")),
                                                            liftCoefficient(config.get<float>("/airplane/liftCoefficient")),
                                                            flyingBrakesDragRatio(config.get<float>("/airplane/flyingBrakesDragRatio")),
                                                            flyingGearDragRatio(config.get<float>("/airplane/flyingGearDragRatio")),
                                                            groundBrakesDragRatio(config.get<float>("/airplane/groundBrakesDragRatio")),
                                                            stallLiftRatio(config.get<float>("/airplane/stallLiftRatio")) {
}

void AircraftPhysics::update(AircraftState &state, const float dt) const {
    state.forces.thrust = state.controls.throttle * engineThrust;
    state.forces.drag = (state.forces.speed * state.forces.speed) * dragCoefficient;
    state.forces.lift = (state.forces.speed * state.forces.speed) * liftCoefficient;


    // fuel consumption
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
        state.fuel = 0.0f;
        state.controls.throttle = 0.0f;
    }

    // the aircraft behavior flying vs driving
    // todo move out?
    if (state.flying) {
        if (state.controls.brakes) state.forces.drag *= flyingBrakesDragRatio; // breaks increase drag by 600%
        if (state.controls.gear) state.forces.drag *= flyingGearDragRatio; // gear generate drag
        if (state.forces.speed < stallSpeed) state.forces.lift *= stallLiftRatio; // stall reduce lift by 90%
    } else {
        if (state.controls.brakes) state.forces.drag *= groundBrakesDragRatio; // on ground, drag mimic the wheels brakes
        const float dampFactor = powf(0.9f, dt * 60.0f); // normalized to 60fps baseline
        if (state.controls.brakes && state.forces.speed < groundBrakesSpeed) state.forces.velocity = state.forces.velocity * dampFactor;
        if (state.forces.velocity.y < 0.0f) state.forces.velocity.y = 0.0f; // on ground there is no more velocity down
    }

    const float mass = weight / 9.81f;

    // forces vectors
    // const auto thrustForce = Vector3Scale(state.orientation.forward, state.forces.thrust);
    const auto thrustForce = state.orientation.forward * state.forces.thrust;
    const auto dragForce = state.orientation.forward * -state.forces.drag;
    const auto liftForce = state.orientation.up * state.forces.lift;
    const auto weightForce = GamePhysics::Gravity * mass;

    const auto total = thrustForce + dragForce + weightForce + liftForce;
    const auto acceleration = total / mass;

    // acceleration
    state.forces.velocity = state.forces.velocity + (acceleration * dt);
    state.forces.speed = Vector3Length(state.forces.velocity);

    // limit velocity (not suppose to happen, the drag should be equal to thrust in max speed)
    if (state.forces.speed > maxSpeed && state.forces.speed != 0.0f) {
        state.forces.velocity = state.forces.velocity * maxSpeed / state.forces.speed;
        state.forces.speed = Vector3Length(state.forces.velocity);
    }

    // weathervaning
    // https://en.wikipedia.org/wiki/Weathervane_effect
    if (state.forces.speed > stallSpeed) {
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

    // keep the center less than SHIFT_THRESHOLD to keep the
    // calculations accurate (large floats issue)
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
