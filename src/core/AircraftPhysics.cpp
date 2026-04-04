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
    // auto forces = state.forces;
    // const auto controls = state.controls;
    // const auto dir = state.orientation;
    state.forces.thrust = state.controls.throttle * engineThrust;
    state.forces.drag = (state.forces.speed * state.forces.speed) * dragCoefficient;
    state.forces.lift = (state.forces.speed * state.forces.speed) * liftCoefficient;

    // the aircraft behavior flying vs driving
    // todo move out?
    if (state.flying) {
        if (state.controls.brakes) state.forces.drag *= flyingBrakesDragRatio; // breaks increase drag by 600%
        if (state.controls.gear) state.forces.drag *= flyingGearDragRatio; // gear generate drag
        if (state.forces.speed < stallSpeed) state.forces.lift *= stallLiftRatio; // stall reduce lift by 90%
    } else {
        if (state.controls.brakes) state.forces.drag *= groundBrakesDragRatio; // on ground, drag mimic the wheels brakes
        if (state.controls.brakes && state.forces.speed < groundBrakesSpeed) state.forces.velocity = Vector3Scale(state.forces.velocity, 0.9f);
        state.forces.lift *= stallLiftRatio; // always stall
        if (state.forces.velocity.y < 0.0f) state.forces.velocity.y = 0.0f; // on ground there is no more velocity down
    }

    const float mass = weight / 9.81f;

    // forces vectors
    const auto thrustForce = Vector3Scale(state.orientation.forward, state.forces.thrust);
    const auto dragForce = Vector3Scale(state.orientation.forward, -state.forces.drag);
    const auto liftForce = Vector3Scale(state.orientation.up, state.forces.lift);
    const auto weightForce = Vector3Scale(GamePhysics::Gravity, mass);

    const auto total = Vector3Add(Vector3Add(Vector3Add(thrustForce, dragForce), weightForce), liftForce);
    const auto acceleration = Vector3Scale(total, 1.0f / mass);

    // acceleration
    state.forces.velocity = Vector3Add(state.forces.velocity, Vector3Scale(acceleration, dt));
    state.forces.speed = Vector3Length(state.forces.velocity);

    // limit velocity
    if (state.forces.speed > maxSpeed && state.forces.speed != 0.0f) {
        state.forces.velocity = Vector3Scale(state.forces.velocity, maxSpeed / state.forces.speed);
        state.forces.speed = Vector3Length(state.forces.velocity);
    }

    // weathervaning
    // https://en.wikipedia.org/wiki/Weathervane_effect
    if (state.forces.speed > stallSpeed) {
        auto [x, y, z] = Vector3Scale(state.orientation.forward, state.forces.speed);
        state.forces.velocity.x = Lerp(state.forces.velocity.x, x, 2.0f * dt);
        state.forces.velocity.y = Lerp(state.forces.velocity.y, y, 2.0f * dt);
        state.forces.velocity.z = Lerp(state.forces.velocity.z, z, 2.0f * dt);
    }
}
