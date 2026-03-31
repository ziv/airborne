#include "AircraftPhysics.h"
#include "../primitives/Constants.h"
#include "raymath.h"

AircraftPhysics::AircraftPhysics(const AppConfig &config) : weight(config.weight),
                                                            engineThrust(config.engineThrust),
                                                            maxSpeed(config.maxSpeed),
                                                            stallSpeed(config.stallSpeed),
                                                            dragCoefficient(config.dragCoefficient),
                                                            liftCoefficient(config.liftCoefficient) {
}

void AircraftPhysics::update(const float dt,
                             const bool flying,
                             const PilotControls &controls,
                             const Directions &dir) {
    thrust = controls.throttle * engineThrust;
    drag = (speed * speed) * dragCoefficient;
    lift = (speed * speed) * liftCoefficient;

    if (flying) {
        // breaks increase drag by 600%
        if (controls.brakes) drag *= 6.0f;
        // gear generate drag
        if (controls.gear) drag *= 1.8f;
        // stall reduce lift by 90%
        if (speed < stallSpeed) lift *= 0.1f;
    } else {
        // on ground, drag mimic the wheels brakes
        if (controls.brakes) drag *= 1000.0f;
        // drag alone is not enough, when we reach the limit, we reduce the speed by force
        if (controls.brakes && speed < 10.0f) velocity = Vector3Scale(velocity, 0.9f);
        // always stall
        lift *= 0.1f;
        // on ground there is no more velocity down
        if (velocity.y < 0.0f) velocity.y = 0.0f;
    }


    const float mass = weight / 9.81f;

    // forces vectors
    const auto thrustForce = Vector3Scale(dir.forward, thrust);
    const auto dragForce = Vector3Scale(dir.forward, -drag);
    const auto liftForce = Vector3Scale(dir.up, lift);
    const auto weightForce = Vector3Scale(GamePhysics::Gravity, mass);

    const auto total = Vector3Add(Vector3Add(Vector3Add(thrustForce, dragForce), weightForce), liftForce);
    const auto acceleration = Vector3Scale(total, 1.0f / mass);

    // acceleration
    velocity = Vector3Add(velocity, Vector3Scale(acceleration, dt));
    speed = Vector3Length(velocity);

    // limit velocity
    if (speed > maxSpeed && speed != 0.0f) {
        velocity = Vector3Scale(velocity, maxSpeed / speed);
        speed = Vector3Length(velocity);
    }

    // weathervaning
    // https://en.wikipedia.org/wiki/Weathervane_effect
    if (speed > stallSpeed) {
        auto [x, y, z] = Vector3Scale(dir.forward, speed);
        velocity.x = Lerp(velocity.x, x, 2.0f * dt);
        velocity.y = Lerp(velocity.y, y, 2.0f * dt);
        velocity.z = Lerp(velocity.z, z, 2.0f * dt);
    }
}
