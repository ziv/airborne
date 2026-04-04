#include "PhysicsEngine.h"

#include "raymath.h"

PhysicsEngine::PhysicsEngine(const AircraftProperties &props) : props(props) {
}

void PhysicsEngine::applyFlying(const AircraftState &state,
                                const float dt,
                                const float dragRatio,
                                const float liftRatio) const {
    const float mass = props.weight / 9.81f;
    const auto dir = state.orientation;
    auto forces = state.forces;
    const auto speedSquare = forces.speed * forces.speed;

    // update forces
    forces.thrust = state.controls.throttle * props.engineThrust;
    forces.drag = speedSquare * props.dragCoefficient * dragRatio;
    forces.lift = speedSquare * props.liftCoefficient * liftRatio;

    // calculate changes
    const auto thrustForce = Vector3Scale(dir.forward, forces.thrust);
    const auto dragForce = Vector3Scale(dir.forward, -forces.drag);
    const auto liftForce = Vector3Scale(dir.up, forces.lift);
    const auto weightForce = Vector3Scale(GamePhysics::Gravity, mass);

    // union of all forces
    const auto total = Vector3Add(Vector3Add(Vector3Add(thrustForce, dragForce), weightForce), liftForce);
    const auto acceleration = Vector3Scale(total, 1.0f / mass);

    // acceleration
    forces.velocity = Vector3Add(forces.velocity, Vector3Scale(acceleration, dt));
    forces.speed = Vector3Length(forces.velocity);

    // limit velocity
    if (forces.speed > props.maxSpeed && forces.speed != 0.0f) {
        forces.velocity = Vector3Scale(forces.velocity, props.maxSpeed / forces.speed);
        forces.speed = Vector3Length(forces.velocity);
    }

    // weathervaning
    // https://en.wikipedia.org/wiki/Weathervane_effect
    if (forces.speed > props.stallSpeed) {
        auto [x, y, z] = Vector3Scale(dir.forward, forces.speed);
        forces.velocity.x = Lerp(forces.velocity.x, x, 2.0f * dt);
        forces.velocity.y = Lerp(forces.velocity.y, y, 2.0f * dt);
        forces.velocity.z = Lerp(forces.velocity.z, z, 2.0f * dt);
    }
}
