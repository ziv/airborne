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

void AircraftPhysics::update(const float dt,
                             const bool flying,
                             const PilotControls &controls,
                             const Directions &dir) {
    forces.thrust = controls.throttle * engineThrust;
    forces.drag = (forces.speed * forces.speed) * dragCoefficient;
    forces.lift = (forces.speed * forces.speed) * liftCoefficient;

    // the aircraft behavior flying vs driving
    if (flying) {
        if (controls.brakes) forces.drag *= flyingBrakesDragRatio; // breaks increase drag by 600%
        if (controls.gear) forces.drag *= flyingGearDragRatio; // gear generate drag
        if (forces.speed < stallSpeed) forces.lift *= stallLiftRatio; // stall reduce lift by 90%
    } else {
        if (controls.brakes) forces.drag *= groundBrakesDragRatio; // on ground, drag mimic the wheels brakes
        if (controls.brakes && forces.speed < groundBrakesSpeed) forces.velocity = Vector3Scale(forces.velocity, 0.9f);
        forces.lift *= stallLiftRatio; // always stall
        if (forces.velocity.y < 0.0f) forces.velocity.y = 0.0f; // on ground there is no more velocity down
    }

    const float mass = weight / 9.81f;

    // forces vectors
    const auto thrustForce = Vector3Scale(dir.forward, forces.thrust);
    const auto dragForce = Vector3Scale(dir.forward, -forces.drag);
    const auto liftForce = Vector3Scale(dir.up, forces.lift);
    const auto weightForce = Vector3Scale(GamePhysics::Gravity, mass);

    const auto total = Vector3Add(Vector3Add(Vector3Add(thrustForce, dragForce), weightForce), liftForce);
    const auto acceleration = Vector3Scale(total, 1.0f / mass);

    // acceleration
    forces.velocity = Vector3Add(forces.velocity, Vector3Scale(acceleration, dt));
    forces.speed = Vector3Length(forces.velocity);

    // limit velocity
    if (forces.speed > maxSpeed && forces.speed != 0.0f) {
        forces.velocity = Vector3Scale(forces.velocity, maxSpeed / forces.speed);
        forces.speed = Vector3Length(forces.velocity);
    }

    // weathervaning
    // https://en.wikipedia.org/wiki/Weathervane_effect
    if (forces.speed > stallSpeed) {
        auto [x, y, z] = Vector3Scale(dir.forward, forces.speed);
        forces.velocity.x = Lerp(forces.velocity.x, x, 2.0f * dt);
        forces.velocity.y = Lerp(forces.velocity.y, y, 2.0f * dt);
        forces.velocity.z = Lerp(forces.velocity.z, z, 2.0f * dt);
    }
}
